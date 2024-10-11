#include "database/database_callbacks.h"
#include "database/database_handler.h"
#include "database/db_column_data.h"
#include "database/db_result.h"
#include "utility/cache.h"
#include "utility/caches.h"
#include "utility/event_data_lite.h"
#include "utility/json_err.h"
#include "utility/perms.h"
#include "utility/response.h"

#include <dpp/channel.h>
#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/guild.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <dpp/permissions.h>
#include <dpp/restresults.h>
#include <dpp/role.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>

#include <atomic>
#include <cstdint>
#include <exception>
#include <format>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

size_t mln::caches::s_saved_select_dump_channel{ 0 };

size_t mln::caches::s_saved_on_guild_member_update{ 0 };
size_t mln::caches::s_saved_on_channel_update{ 0 };
size_t mln::caches::s_saved_on_user_update{ 0 };
size_t mln::caches::s_saved_on_guild_update{ 0 };
size_t mln::caches::s_saved_on_guild_role_update{ 0 };

std::atomic_ullong mln::caches::s_cache_misses{ 0 };
std::atomic_ullong mln::caches::s_cache_requests{ 0 };
dpp::cluster* mln::caches::s_cluster{ nullptr };
mln::database_handler* mln::caches::s_db{ nullptr };

mln::cache_primitive<uint64_t, uint64_t, 10000, 1000, 0.75, true> mln::caches::dump_channels_cache{};
mln::cache<uint64_t, std::vector<std::string>, false, 400, 30, 0.7, true> mln::caches::show_all_cache{};
mln::cache<std::tuple<uint64_t, uint64_t>, std::vector<std::string>, false, 1000, 100, 0.7, true, mln::caches::composite_tuple_hash, mln::caches::composite_tuple_eq> mln::caches::show_user_cache{};
mln::cache<uint64_t, dpp::guild, false, 3000, 300, 0.7, true> mln::caches::guild_cache{};
mln::cache<uint64_t, dpp::channel, false, 4000, 300, 0.7, true> mln::caches::channel_cache{};
mln::cache<uint64_t, dpp::user_identified, false, 6000, 500, 0.7, true> mln::caches::user_cache{};
mln::cache<std::tuple<uint64_t, uint64_t>, dpp::guild_member, false, 6000, 500, 0.7, true, mln::caches::composite_tuple_hash, mln::caches::composite_tuple_eq> mln::caches::member_cache{};
mln::cache<uint64_t, dpp::role, false, 6000, 500, 0.7, true> mln::caches::role_cache{};

unsigned long long mln::caches::get_total_cache_requests() {
	return mln::caches::s_cache_requests.load(std::memory_order_relaxed);
}
unsigned long long mln::caches::get_total_cache_misses() {
	return mln::caches::s_cache_misses.load(std::memory_order_relaxed);
}
long double mln::caches::get_cache_misses_rate() {
    const unsigned long long requests = mln::caches::s_cache_requests.load(std::memory_order_relaxed);
    const long double misses = static_cast<long double>(mln::caches::s_cache_misses.load(std::memory_order_relaxed));
    if (requests == 0) [[unlikely]] {
        return 0.0;
    }
    return misses / static_cast<long double>(requests);
}
void mln::caches::cleanup() {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        return;
    }

    mln::caches::s_cluster->on_guild_member_update.detach(mln::caches::s_saved_on_guild_member_update);
    mln::caches::s_cluster->on_channel_update.detach(mln::caches::s_saved_on_channel_update);
    mln::caches::s_cluster->on_user_update.detach(mln::caches::s_saved_on_user_update);
    mln::caches::s_cluster->on_guild_update.detach(mln::caches::s_saved_on_guild_update);
    mln::caches::s_cluster->on_guild_role_update.detach(mln::caches::s_saved_on_guild_role_update);
    mln::caches::s_saved_on_guild_member_update = 0;
    mln::caches::s_saved_on_channel_update = 0;
    mln::caches::s_saved_on_user_update = 0;
    mln::caches::s_saved_on_guild_update = 0;
    mln::caches::s_saved_on_guild_role_update = 0;

    s_db->delete_statement(s_saved_select_dump_channel);
    mln::caches::s_saved_select_dump_channel = 0;

    mln::caches::s_cluster = nullptr;
    mln::caches::s_db = nullptr;
    mln::caches::s_cache_misses.store(0, std::memory_order_relaxed);
    mln::caches::s_cache_requests.store(0, std::memory_order_relaxed);

    mln::caches::dump_channels_cache.clear();
    mln::caches::show_all_cache.clear();
    mln::caches::show_user_cache.clear();
    mln::caches::guild_cache.clear();
    mln::caches::channel_cache.clear();
    mln::caches::user_cache.clear();
    mln::caches::member_cache.clear();
    mln::caches::role_cache.clear();
}
bool mln::caches::is_initialized() {
    return mln::caches::s_cluster != nullptr && mln::caches::s_db != nullptr;
}
void mln::caches::init(dpp::cluster* cluster, database_handler* db) {
    if (mln::caches::is_initialized()) [[unlikely]] {
        mln::caches::cleanup();
    }

    mln::caches::s_cluster = cluster;
    mln::caches::s_db = db;
    mln::caches::s_cache_misses.store(0, std::memory_order_relaxed);
    mln::caches::s_cache_requests.store(0, std::memory_order_relaxed);

    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("Failed to initialize caches, either the cluster or the database are not valid references!");
    }

    //Note that some of the info in the events will not be filled if the cache policy of the cluster is ::cp_none (dpp does dpp::find_X for some stuff, which will be empty)
    s_saved_on_guild_member_update = s_cluster->on_guild_member_update([](const dpp::guild_member_update_t& event_data) {
        if (!mln::caches::is_initialized()) [[unlikely]] {
            throw std::exception("Failed to initialize caches, either the cluster or the database are not valid references!");
        }

        const bool success = mln::caches::member_cache.update_element(std::make_tuple(event_data.updated.guild_id, event_data.updated.user_id), 
            event_data.updated).has_value();

        s_cluster->log(dpp::loglevel::ll_debug, success ? 
            std::format("Updated guild member [{}] from guild [{}].", static_cast<uint64_t>(event_data.updated.user_id), static_cast<uint64_t>(event_data.updated.guild_id)) :
            std::format("Not updated guild member [{}] from guild [{}].", static_cast<uint64_t>(event_data.updated.user_id), static_cast<uint64_t>(event_data.updated.guild_id)));
        });

    s_saved_on_channel_update = s_cluster->on_channel_update([](const dpp::channel_update_t& event_data) { 
        if (event_data.updated != nullptr) [[likely]] {
            if (!mln::caches::is_initialized()) [[unlikely]] {
                throw std::exception("Failed to initialize caches, either the cluster or the database are not valid references!");
            }

            const dpp::channel& channel = *event_data.updated;
            const bool success = mln::caches::channel_cache.update_element(channel.id, channel).has_value();

            s_cluster->log(dpp::loglevel::ll_debug, success ?
                std::format("Updated channel [{}] from guild [{}].", static_cast<uint64_t>(channel.id), static_cast<uint64_t>(channel.guild_id)) :
                std::format("Not updated channel [{}] from guild [{}].", static_cast<uint64_t>(channel.id), static_cast<uint64_t>(channel.guild_id)));
        }
        });

    s_saved_on_user_update = s_cluster->on_user_update([](const dpp::user_update_t& event_data) { 
        if (!mln::caches::is_initialized()) [[unlikely]] {
            throw std::exception("Failed to initialize caches, either the cluster or the database are not valid references!");
        }

        const bool success = mln::caches::user_cache.update_element(event_data.updated.id, event_data.updated).has_value();

        s_cluster->log(dpp::loglevel::ll_debug, success ?
            std::format("Updated user [{}].", static_cast<uint64_t>(event_data.updated.id)) :
            std::format("Not updated user [{}].", static_cast<uint64_t>(event_data.updated.id)));
        });

    s_saved_on_guild_update = s_cluster->on_guild_update([](const dpp::guild_update_t& event_data) { 
        if (event_data.updated != nullptr) [[likely]] {
            if (!mln::caches::is_initialized()) [[unlikely]] {
                throw std::exception("Failed to initialize caches, either the cluster or the database are not valid references!");
            }

            const dpp::guild& guild = *event_data.updated;
            const bool success = mln::caches::guild_cache.update_element(guild.id, guild).has_value();

            s_cluster->log(dpp::loglevel::ll_debug, success ?
                std::format("Updated guild [{}].", static_cast<uint64_t>(guild.id)) :
                std::format("Not updated guild [{}].", static_cast<uint64_t>(guild.id)));
        }
        });

    s_saved_on_guild_role_update = s_cluster->on_guild_role_update([](const dpp::guild_role_update_t& event_data) { 
        if (event_data.updated != nullptr) [[likely]] {
            if (!mln::caches::is_initialized()) [[unlikely]] {
                throw std::exception("Failed to initialize caches, either the cluster or the database are not valid references!");
            }

            const dpp::role& role = *event_data.updated;
            const bool success = mln::caches::role_cache.update_element(role.id, role).has_value();

            s_cluster->log(dpp::loglevel::ll_debug, success ?
                std::format("Updated role [{}] from guild [{}].", static_cast<uint64_t>(role.id), static_cast<uint64_t>(role.guild_id)) :
                std::format("Not updated role [{}] from guild [{}].", static_cast<uint64_t>(role.id), static_cast<uint64_t>(role.guild_id)));
        }
        });

    const mln::db_result_t res = s_db->save_statement("SELECT dedicated_channel_id FROM guild_profile WHERE guild_id = ?1;", s_saved_select_dump_channel);
    if (res.type != mln::db_result::ok) [[unlikely]] {
        const std::string err_msg = std::format("An error occurred while saving the select dump channel stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res.type), res.err_text);
        throw std::exception(err_msg.c_str());
    }
}


std::optional<uint64_t> mln::caches::get_dump_channel_id(const uint64_t guild_id) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }

    if (guild_id == 0) [[unlikely]] {
        return std::nullopt;
    }

	mln::caches::s_cache_requests.fetch_add(1, std::memory_order_relaxed);

    //Look in cache
	std::optional<uint64_t> result = mln::caches::dump_channels_cache.get_element(guild_id);
	if (result.has_value()) [[likely]] {
		return result;
	}

	mln::caches::s_cache_misses.fetch_add(1, std::memory_order_relaxed);

    //Look in database
    const db_result_t res = s_db->bind_parameter(s_saved_select_dump_channel, 0, 1, static_cast<int64_t>(guild_id));
    if (res.type != mln::db_result::ok) [[unlikely]] {
        s_cluster->log(dpp::loglevel::ll_error, std::format("Failed to bind query parameters for select dump channel! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res.type), res.err_text));
        return std::nullopt;
    }

    mln::database_callbacks_t calls{};
    calls.callback_data = static_cast<void*>(&result);
    calls.type_definer_callback = [](void*, int) {return false;};
    calls.data_adder_callback = [](void* d, int col, mln::db_column_data_t&& c_data) {
        std::optional<uint64_t>* opt_ptr = static_cast<std::optional<uint64_t>*>(d);

        *opt_ptr = static_cast<uint64_t>(std::get<int64_t>(c_data.data));
    };

    const db_result_t exec_res = s_db->exec(s_saved_select_dump_channel, calls);
    if (mln::database_handler::is_exec_error(exec_res.type)) [[unlikely]] {
        s_cluster->log(dpp::loglevel::ll_error, std::format("Failed to execute query for select dump channel! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(exec_res.type), exec_res.err_text));
    }

    //If value found in database, store it in cache and return it
    if (result.has_value()) [[likely]] {
        return mln::caches::dump_channels_cache.add_element(guild_id, result.value());
    }   

	return std::nullopt;
}


dpp::task<std::optional<dpp::message>> mln::caches::get_message_task(const uint64_t message_id, const uint64_t channel_id, const dpp::permission bot_permissions, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::message>* const resolved_map) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }

    if (message_id == 0) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve msg with id [{}] from channel [{}]! Invalid msg/channel id!", message_id, channel_id);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    if (!mln::perms::check_permissions(bot_permissions, dpp::permissions::p_view_channel | dpp::permissions::p_read_message_history)) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve msg with id [{}] from channel [{}]! Missing permissions!", message_id, channel_id);

        co_await mln::response::co_respond(lite_data, err_text, false, err_text);

        co_return std::nullopt;
    }

    if (resolved_map) {
        const std::map<dpp::snowflake, dpp::message>::const_iterator it = resolved_map->find(message_id);
        if (it != resolved_map->end()) {
            co_return it->second;
        }
    }

    if (channel_id == 0) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve msg with id [{}] from channel [{}]! Invalid channel id!", message_id, channel_id);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    const dpp::confirmation_callback_t result = co_await lite_data.creator->co_message_get(message_id, channel_id);
    if (result.is_error()) [[unlikely]] {
        const dpp::error_info err = result.get_error();
        const std::string err_text = std::format("Failed to retrieve msg with id [{}] from channel [{}]! Error: [{}], details: [{}].", message_id, channel_id, mln::get_json_err_text(err.code), err.human_readable);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    if (!std::holds_alternative<dpp::message>(result.value)) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve msg with id [{}] from channel [{}]! Failed to retrieve message from Discord.", message_id, channel_id);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    co_return std::get<dpp::message>(result.value);
}
std::optional<dpp::message> mln::caches::get_message(const uint64_t message_id, const uint64_t channel_id, const dpp::permission bot_permissions, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::message>* const resolved_map) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }

    dpp::cluster& bot = lite_data.creator ? *lite_data.creator : *mln::caches::s_cluster;

    if (message_id == 0) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve msg with id [{}] from channel [{}]! Invalid msg/channel id!", message_id, channel_id);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    if (!mln::perms::check_permissions(bot_permissions, dpp::permissions::p_view_channel | dpp::permissions::p_read_message_history)) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve msg with id [{}] from channel [{}]! Missing permissions!", message_id, channel_id);

        mln::response::respond(lite_data, err_text, false, err_text);

        return std::nullopt;
    }

    if (resolved_map) {
        const std::map<dpp::snowflake, dpp::message>::const_iterator it = resolved_map->find(message_id);
        if (it != resolved_map->end()) {
            return it->second;
        }
    }

    if (channel_id == 0) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve msg with id [{}] from channel [{}]! Invalid channel id!", message_id, channel_id);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    dpp::confirmation_callback_t result;
    lite_data.creator->message_get(message_id, channel_id, [&result](const dpp::confirmation_callback_t& conf) { result = conf; });
    if (result.is_error()) [[unlikely]] {
        const dpp::error_info err = result.get_error();
        const std::string err_text = std::format("Failed to retrieve msg with id [{}] from channel [{}]! Error: [{}], details: [{}].", message_id, channel_id, mln::get_json_err_text(err.code), err.human_readable);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    if (!std::holds_alternative<dpp::message>(result.value)) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve msg with id [{}] from channel [{}]! Failed to retrieve message from Discord.", message_id, channel_id);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    return std::get<dpp::message>(result.value);
}


std::optional<std::shared_ptr<const std::vector<std::string>>> mln::caches::get_show_all(const uint64_t guild_id) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }
    
    if (guild_id == 0) [[unlikely]] {
        return std::nullopt;
    }

    mln::caches::s_cache_requests.fetch_add(1, std::memory_order_relaxed);

    //Look in cache
    std::optional<std::shared_ptr<const std::vector<std::string>>> result = mln::caches::show_all_cache.get_element(guild_id);
    if (!result.has_value()) [[unlikely]] {
        mln::caches::s_cache_misses.fetch_add(1, std::memory_order_relaxed);
    }

    return std::nullopt;
}
std::optional<std::shared_ptr<const std::vector<std::string>>> mln::caches::get_show_user(const uint64_t guild_id, const uint64_t user_id) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }
    
    if (guild_id == 0 || user_id == 0) [[unlikely]] {
        return std::nullopt;
    }

    mln::caches::s_cache_requests.fetch_add(1, std::memory_order_relaxed);

    //Look in cache
    std::optional<std::shared_ptr<const std::vector<std::string>>> result = mln::caches::show_user_cache.get_element({guild_id, user_id});
    if (!result.has_value()) {
        mln::caches::s_cache_misses.fetch_add(1, std::memory_order_relaxed);
    }

    return std::nullopt;
}


dpp::task<std::optional<std::shared_ptr<const dpp::guild>>> mln::caches::get_guild_task(const uint64_t guild_id, event_data_lite_t& lite_data) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }
    
    if (guild_id == 0) [[unlikely]] {
        static const std::string s_err_text = "Failed to retrieve guild! Invalid guild id!";

        co_await mln::response::co_respond(lite_data, s_err_text, true, s_err_text);

        co_return std::nullopt;
    }

    mln::caches::s_cache_requests.fetch_add(1, std::memory_order_relaxed);

    //Look in cache
    std::optional<std::shared_ptr<const dpp::guild>> result_opt = mln::caches::guild_cache.get_element(guild_id);
    if (result_opt.has_value()) {
        co_return result_opt;
    }

    mln::caches::s_cache_misses.fetch_add(1, std::memory_order_relaxed);

    const dpp::confirmation_callback_t confirmation = co_await s_cluster->co_guild_get(guild_id);
    if (confirmation.is_error()) [[unlikely]] {
        const dpp::error_info err = confirmation.get_error();
        const std::string err_text = std::format("Failed to retrieve guild! Guild id: [{}], error: [{}], details: [{}].", 
            static_cast<uint64_t>(guild_id), mln::get_json_err_text(err.code), err.human_readable);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }
    
    if (!std::holds_alternative<dpp::guild>(confirmation.value)) {
        const std::string err_text = std::format("Failed to retrieve guild from Discord! Guild id: [{}].", static_cast<uint64_t>(guild_id));

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    co_return mln::caches::guild_cache.add_element(guild_id, std::get<dpp::guild>(confirmation.value));
}
std::optional<std::shared_ptr<const dpp::guild>> mln::caches::get_guild(const uint64_t guild_id, event_data_lite_t& lite_data) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }

    if (guild_id == 0) [[unlikely]] {
        static const std::string s_err_text = "Failed to retrieve guild! Invalid guild id!";

        mln::response::respond(lite_data, s_err_text, true, s_err_text);

        return std::nullopt;
    }

    mln::caches::s_cache_requests.fetch_add(1, std::memory_order_relaxed);

    //Look in cache
    std::optional<std::shared_ptr<const dpp::guild>> result_opt = mln::caches::guild_cache.get_element(guild_id);
    if (result_opt.has_value()) {
        return result_opt;
    }

    mln::caches::s_cache_misses.fetch_add(1, std::memory_order_relaxed);

    dpp::confirmation_callback_t confirmation;
    lite_data.creator->guild_get(guild_id, [&confirmation](const dpp::confirmation_callback_t& conf) { confirmation = conf; });
    if (confirmation.is_error()) [[unlikely]] {
        const dpp::error_info err = confirmation.get_error();
        const std::string err_text = std::format("Failed to retrieve guild! Guild id: [{}], error: [{}], details: [{}].",
            static_cast<uint64_t>(guild_id), mln::get_json_err_text(err.code), err.human_readable);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    if (!std::holds_alternative<dpp::guild>(confirmation.value)) {
        const std::string err_text = std::format("Failed to retrieve guild from Discord! Guild id: [{}].", static_cast<uint64_t>(guild_id));

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    return mln::caches::guild_cache.add_element(guild_id, std::get<dpp::guild>(confirmation.value));
}


dpp::task<std::optional<std::shared_ptr<const dpp::channel>>> mln::caches::get_channel_task(const uint64_t channel_id, event_data_lite_t& lite_data, const dpp::channel* const event_channel, const std::map<dpp::snowflake, dpp::channel>* const resolved_map) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }
    
    if (channel_id == 0) [[unlikely]] {
        static const std::string s_err_text = "Failed to retrieve channel! Invalid channel id!";

        co_await mln::response::co_respond(lite_data, s_err_text, true, s_err_text);

        co_return std::nullopt;
    }

    mln::caches::s_cache_requests.fetch_add(1, std::memory_order_relaxed);

    //Look in cache
    std::optional<std::shared_ptr<const dpp::channel>> result_opt = mln::caches::channel_cache.get_element(channel_id);
    if (result_opt.has_value()) {
        co_return result_opt;
    }

    mln::caches::s_cache_misses.fetch_add(1, std::memory_order_relaxed);

    //Look in resolved cache
    if (event_channel) {
        if (event_channel->id == channel_id) {
            co_return mln::caches::channel_cache.add_element(channel_id, *event_channel);
        }
    }

    if (resolved_map) {
        const std::map<dpp::snowflake, dpp::channel>::const_iterator it = resolved_map->find(channel_id);
        if (it != resolved_map->end()) {
            co_return mln::caches::channel_cache.add_element(channel_id, it->second);
        }
    }

    const dpp::confirmation_callback_t confirmation = co_await s_cluster->co_channel_get(channel_id);
    if (confirmation.is_error()) [[unlikely]] {
        const dpp::error_info err = confirmation.get_error();
        const std::string err_text = std::format("Failed to retrieve channel! channel id: [{}], error: [{}], details: [{}].",
            static_cast<uint64_t>(channel_id), mln::get_json_err_text(err.code), err.human_readable);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    if (!std::holds_alternative<dpp::channel>(confirmation.value)) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve channel from Discord! channel id: [{}].", static_cast<uint64_t>(channel_id));

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    co_return mln::caches::channel_cache.add_element(channel_id, std::get<dpp::channel>(confirmation.value));
}
std::optional<std::shared_ptr<const dpp::channel>> mln::caches::get_channel(const uint64_t channel_id, event_data_lite_t& lite_data, const dpp::channel* const event_channel, const std::map<dpp::snowflake, dpp::channel>* const resolved_map) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }

    if (channel_id == 0) [[unlikely]] {
        static const std::string s_err_text = "Failed to retrieve channel! Invalid channel id!";

        mln::response::respond(lite_data, s_err_text, true, s_err_text);

        return std::nullopt;
    }

    mln::caches::s_cache_requests.fetch_add(1, std::memory_order_relaxed);

    //Look in cache
    std::optional<std::shared_ptr<const dpp::channel>> result_opt = mln::caches::channel_cache.get_element(channel_id);
    if (result_opt.has_value()) {
        return result_opt;
    }

    mln::caches::s_cache_misses.fetch_add(1, std::memory_order_relaxed);

    //Look in resolved cache
    if (event_channel) {
        if (event_channel->id == channel_id) {
            return mln::caches::channel_cache.add_element(channel_id, *event_channel);
        }
    }

    if (resolved_map) {
        const std::map<dpp::snowflake, dpp::channel>::const_iterator it = resolved_map->find(channel_id);
        if (it != resolved_map->end()) {
            return mln::caches::channel_cache.add_element(channel_id, it->second);
        }
    }

    dpp::confirmation_callback_t confirmation;
    lite_data.creator->channel_get(channel_id, [&confirmation](const dpp::confirmation_callback_t& conf) { confirmation = conf; });
    if (confirmation.is_error()) [[unlikely]] {
        const dpp::error_info err = confirmation.get_error();
        const std::string err_text = std::format("Failed to retrieve channel! channel id: [{}], error: [{}], details: [{}].",
            static_cast<uint64_t>(channel_id), mln::get_json_err_text(err.code), err.human_readable);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    if (!std::holds_alternative<dpp::channel>(confirmation.value)) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve channel from Discord! channel id: [{}].", static_cast<uint64_t>(channel_id));

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    return mln::caches::channel_cache.add_element(channel_id, std::get<dpp::channel>(confirmation.value));
}


dpp::task<std::optional<std::shared_ptr<const dpp::user_identified>>> mln::caches::get_user_task(const uint64_t user_id, event_data_lite_t& lite_data, const dpp::user* const invoking_usr, const std::map<dpp::snowflake, dpp::user>* const resolved_map) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }

    if (user_id == 0) [[unlikely]] {
        static const std::string s_err_text = "Failed to retrieve user! Invalid user id!";

        co_await mln::response::co_respond(lite_data, s_err_text, true, s_err_text);

        co_return std::nullopt;
    }

    mln::caches::s_cache_requests.fetch_add(1, std::memory_order_relaxed);

    //Look in cache
    std::optional<std::shared_ptr<const dpp::user_identified>> result_opt = mln::caches::user_cache.get_element(user_id);
    if (result_opt.has_value()) {
        co_return result_opt;
    }

    mln::caches::s_cache_misses.fetch_add(1, std::memory_order_relaxed);

    //Look in resolved cache
    if (invoking_usr) {
        if (invoking_usr->id == user_id) {
            co_return mln::caches::user_cache.add_element(user_id, *invoking_usr);
        }
    }

    if (resolved_map) {
        const std::map<dpp::snowflake, dpp::user>::const_iterator it = resolved_map->find(user_id);
        if (it != resolved_map->end()) {
            co_return mln::caches::user_cache.add_element(user_id, it->second);
        }
    }

    const dpp::confirmation_callback_t confirmation = co_await s_cluster->co_user_get(user_id);
    if (confirmation.is_error()) [[unlikely]] {
        const dpp::error_info err = confirmation.get_error();
        const std::string err_text = std::format("Failed to retrieve user! user id: [{}], error: [{}], details: [{}].",
            static_cast<uint64_t>(user_id), mln::get_json_err_text(err.code), err.human_readable);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    if (!std::holds_alternative<dpp::user_identified>(confirmation.value)) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve user from Discord! user id: [{}].", static_cast<uint64_t>(user_id));

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    co_return mln::caches::user_cache.add_element(user_id, std::get<dpp::user_identified>(confirmation.value));
}
std::optional<std::shared_ptr<const dpp::user_identified>> mln::caches::get_user(const uint64_t user_id, event_data_lite_t& lite_data, const dpp::user* const invoking_usr, const std::map<dpp::snowflake, dpp::user>* const resolved_map) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }

    if (user_id == 0) [[unlikely]] {
        static const std::string s_err_text = "Failed to retrieve user! Invalid user id!";

        mln::response::respond(lite_data, s_err_text, true, s_err_text);

        return std::nullopt;
    }

    mln::caches::s_cache_requests.fetch_add(1, std::memory_order_relaxed);

    //Look in cache
    std::optional<std::shared_ptr<const dpp::user_identified>> result_opt = mln::caches::user_cache.get_element(user_id);
    if (result_opt.has_value()) {
        return result_opt;
    }

    mln::caches::s_cache_misses.fetch_add(1, std::memory_order_relaxed);

    //Look in resolved cache
    if (invoking_usr) {
        if (invoking_usr->id == user_id) {
            return mln::caches::user_cache.add_element(user_id, *invoking_usr);
        }
    }

    if (resolved_map) {
        const std::map<dpp::snowflake, dpp::user>::const_iterator it = resolved_map->find(user_id);
        if (it != resolved_map->end()) {
            return mln::caches::user_cache.add_element(user_id, it->second);
        }
    }

    dpp::confirmation_callback_t confirmation;
    lite_data.creator->user_get(user_id, [&confirmation](const dpp::confirmation_callback_t& conf) { confirmation = conf; });
    if (confirmation.is_error()) [[unlikely]] {
        const dpp::error_info err = confirmation.get_error();
        const std::string err_text = std::format("Failed to retrieve user! user id: [{}], error: [{}], details: [{}].",
            static_cast<uint64_t>(user_id), mln::get_json_err_text(err.code), err.human_readable);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    if (!std::holds_alternative<dpp::user_identified>(confirmation.value)) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve user from Discord! user id: [{}].", static_cast<uint64_t>(user_id));

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    return mln::caches::user_cache.add_element(user_id, std::get<dpp::user_identified>(confirmation.value));
}


dpp::task<std::optional<std::shared_ptr<const dpp::guild_member>>> mln::caches::get_member_task(const uint64_t guild_id, const uint64_t user_id, event_data_lite_t& lite_data, const dpp::guild_member* const invoking_usr, const std::map<dpp::snowflake, dpp::guild_member>* const resolved_map) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }

    if (guild_id == 0 || user_id == 0) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve guild member with id [{}] from guild [{}]! Invalid guild/user id!", user_id, guild_id);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    mln::caches::s_cache_requests.fetch_add(1, std::memory_order_relaxed);

    const std::tuple<uint64_t, uint64_t> guild_user_ids{ guild_id, user_id };
    //Look in cache
    std::optional<std::shared_ptr<const dpp::guild_member>> result_opt = mln::caches::member_cache.get_element(guild_user_ids);
    if (result_opt.has_value()) {
        co_return result_opt;
    }

    mln::caches::s_cache_misses.fetch_add(1, std::memory_order_relaxed);

    //Look in resolved cache
    if (invoking_usr) {
        if (invoking_usr->user_id == user_id) {
            co_return mln::caches::member_cache.add_element(guild_user_ids, *invoking_usr);
        }
    }

    if (resolved_map) {
        const std::map<dpp::snowflake, dpp::guild_member>::const_iterator it = resolved_map->find(user_id);
        if (it != resolved_map->end()) {
            co_return mln::caches::member_cache.add_element(guild_user_ids, it->second);
        }
    }

    const dpp::confirmation_callback_t confirmation = co_await s_cluster->co_guild_get_member(std::get<0>(guild_user_ids), std::get<1>(guild_user_ids));
    if (confirmation.is_error()) [[unlikely]] {
        const dpp::error_info err = confirmation.get_error();
        const std::string err_text = std::format("Failed to retrieve guild member! guild id: [{}], user id: [{}], error: [{}], details: [{}].",
            guild_id, user_id, mln::get_json_err_text(err.code), err.human_readable);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    if (!std::holds_alternative<dpp::guild_member>(confirmation.value)) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve guild member from Discord! guild id: [{}], user id: [{}].", guild_id, user_id);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    co_return mln::caches::member_cache.add_element(guild_user_ids, std::get<dpp::guild_member>(confirmation.value));
}
std::optional<std::shared_ptr<const dpp::guild_member>> mln::caches::get_member(const uint64_t guild_id, const uint64_t user_id, event_data_lite_t& lite_data, const dpp::guild_member* const invoking_usr, const std::map<dpp::snowflake, dpp::guild_member>* const resolved_map) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }

    if (guild_id == 0 || user_id == 0) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve guild member with id [{}] from guild [{}]! Invalid guild/user id!", user_id, guild_id);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    mln::caches::s_cache_requests.fetch_add(1, std::memory_order_relaxed);

    const std::tuple<uint64_t, uint64_t> guild_user_ids{ guild_id, user_id };
    //Look in cache
    std::optional<std::shared_ptr<const dpp::guild_member>> result_opt = mln::caches::member_cache.get_element(guild_user_ids);
    if (result_opt.has_value()) {
        return result_opt;
    }

    mln::caches::s_cache_misses.fetch_add(1, std::memory_order_relaxed);

    //Look in resolved cache
    if (invoking_usr) {
        if (invoking_usr->user_id == user_id) {
            return mln::caches::member_cache.add_element(guild_user_ids, *invoking_usr);
        }
    }

    if (resolved_map) {
        const std::map<dpp::snowflake, dpp::guild_member>::const_iterator it = resolved_map->find(user_id);
        if (it != resolved_map->end()) {
            return mln::caches::member_cache.add_element(guild_user_ids, it->second);
        }
    }

    dpp::confirmation_callback_t confirmation;
    lite_data.creator->guild_get_member(std::get<0>(guild_user_ids), std::get<1>(guild_user_ids), [&confirmation](const dpp::confirmation_callback_t& conf) { confirmation = conf; });
    if (confirmation.is_error()) [[unlikely]] {
        const dpp::error_info err = confirmation.get_error();
        const std::string err_text = std::format("Failed to retrieve guild member! guild id: [{}], user id: [{}], error: [{}], details: [{}].",
            guild_id, user_id, mln::get_json_err_text(err.code), err.human_readable);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    if (!std::holds_alternative<dpp::guild_member>(confirmation.value)) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve guild member from Discord! guild id: [{}], user id: [{}].", guild_id, user_id);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    return mln::caches::member_cache.add_element(guild_user_ids, std::get<dpp::guild_member>(confirmation.value));
}


dpp::task<std::optional<std::shared_ptr<const dpp::role>>> mln::caches::get_role_task(const uint64_t guild_id, const uint64_t role_id, const bool add_all_guild_roles, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::role>* const resolved_map) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }

    if (guild_id == 0 || role_id == 0) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve role with id [{}] from guild [{}]! Invalid role/guild id!", role_id, guild_id);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    mln::caches::s_cache_requests.fetch_add(1, std::memory_order_relaxed);

    //Look in cache
    std::optional<std::shared_ptr<const dpp::role>> result_opt = mln::caches::role_cache.get_element(role_id);
    if (result_opt.has_value()) {
        co_return result_opt;
    }

    mln::caches::s_cache_misses.fetch_add(1, std::memory_order_relaxed);

    //Look in resolved cache
    if (resolved_map) {
        const std::map<dpp::snowflake, dpp::role>::const_iterator it = resolved_map->find(role_id);
        if (it != resolved_map->end()) {
            co_return mln::caches::role_cache.add_element(role_id, it->second);
        }
    }

    const dpp::confirmation_callback_t confirmation = co_await s_cluster->co_roles_get(guild_id);
    if (confirmation.is_error()) [[unlikely]] {
        const dpp::error_info err = confirmation.get_error();
        const std::string err_text = std::format("Failed to retrieve role! guild id: [{}], role id: [{}], error: [{}], details: [{}].",
            guild_id, role_id, mln::get_json_err_text(err.code), err.human_readable);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    if (!std::holds_alternative<dpp::role_map>(confirmation.value)) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve role from Discord! guild id: [{}], role id: [{}].", guild_id, role_id);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    //co_return mln::caches::role_cache.add_element(role_id, std::get<dpp::role>(confirmation.value));

    dpp::role_map map = confirmation.get<dpp::role_map>();
    //Add all the roles found if requested, otherwise add only the requested role
    if (add_all_guild_roles) {
        for (const std::pair<dpp::snowflake, dpp::role>& role_pair : map) {
            mln::caches::role_cache.add_element(role_pair.first, role_pair.second);
        }

        co_return mln::caches::role_cache.get_element(role_id);
    }

    const std::unordered_map<dpp::snowflake, dpp::role>::const_iterator it = map.find(role_id);
    if (it == map.end()) {
        const std::string err_text = std::format("Failed to retrieve role from Discord! Missing role from discord answer! guild id: [{}], role id: [{}].", guild_id, role_id);

        co_await mln::response::co_respond(lite_data, err_text, true, err_text);

        co_return std::nullopt;
    }

    co_return mln::caches::role_cache.add_element(role_id, it->second);
}
std::optional<std::shared_ptr<const dpp::role>> mln::caches::get_role(const uint64_t guild_id, const uint64_t role_id, const bool add_all_guild_roles, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::role>* const resolved_map) {
    if (!mln::caches::is_initialized()) [[unlikely]] {
        throw std::exception("The caches were used before their initialization!");
    }

    if (guild_id == 0 || role_id == 0) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve role with id [{}] from guild [{}]! Invalid role/guild id!", role_id, guild_id);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    mln::caches::s_cache_requests.fetch_add(1, std::memory_order_relaxed);

    //Look in cache
    std::optional<std::shared_ptr<const dpp::role>> result_opt = mln::caches::role_cache.get_element(role_id);
    if (result_opt.has_value()) {
        return result_opt;
    }

    mln::caches::s_cache_misses.fetch_add(1, std::memory_order_relaxed);

    //Look in resolved cache
    if (resolved_map) {
        const std::map<dpp::snowflake, dpp::role>::const_iterator it = resolved_map->find(role_id);
        if (it != resolved_map->end()) {
            return mln::caches::role_cache.add_element(role_id, it->second);
        }
    }

    dpp::confirmation_callback_t confirmation;
    lite_data.creator->roles_get(guild_id, [&confirmation](const dpp::confirmation_callback_t& conf) { confirmation = conf; });
    if (confirmation.is_error()) [[unlikely]] {
        const dpp::error_info err = confirmation.get_error();
        const std::string err_text = std::format("Failed to retrieve role! guild id: [{}], role id: [{}], error: [{}], details: [{}].",
            guild_id, role_id, mln::get_json_err_text(err.code), err.human_readable);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    if (!std::holds_alternative<dpp::role_map>(confirmation.value)) [[unlikely]] {
        const std::string err_text = std::format("Failed to retrieve role from Discord! guild id: [{}], role id: [{}].", guild_id, role_id);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    dpp::role_map map = confirmation.get<dpp::role_map>();
    //Add all the roles found if requested, otherwise add only the requested role
    if (add_all_guild_roles) {
        for (const std::pair<dpp::snowflake, dpp::role>& role_pair : map) {
            mln::caches::role_cache.add_element(role_pair.first, role_pair.second);
        }

        return mln::caches::role_cache.get_element(role_id);
    }

    const std::unordered_map<dpp::snowflake, dpp::role>::const_iterator it = map.find(role_id);
    if (it == map.end()) {
        const std::string err_text = std::format("Failed to retrieve role from Discord! Missing role from discord answer! guild id: [{}], role id: [{}].", guild_id, role_id);

        mln::response::respond(lite_data, err_text, true, err_text);

        return std::nullopt;
    }

    return mln::caches::role_cache.add_element(role_id, it->second);
}
