#include "utility/caches.h"
#include "database/database_handler.h"

#include <dpp/appcommand.h>

static size_t s_saved_select_dump_channel{0};
static size_t s_saved_on_guild_create_id{0};

size_t mln::caches::composite_tuple_hash::operator()(const std::tuple<uint64_t, uint64_t>& key) const {

	std::size_t h1 = std::hash<uint64_t>{}(std::get<0>(key));
	std::size_t h2 = std::hash<uint64_t>{}(std::get<1>(key));

	// Combine the hash values
	return h1 ^ (h2 << 1);
}
unsigned long long mln::caches::get_total_cache_requests() {
	return mln::caches::s_cache_requests;
}
unsigned long long mln::caches::get_total_cache_misses() {
	return mln::caches::s_cache_misses;
}
double mln::caches::get_cache_misses_rate() {
    if (mln::caches::s_cache_requests == 0) {
        return 0.0;
    }
    return static_cast<double>(mln::caches::s_cache_misses) / mln::caches::s_cache_requests;
}
void mln::caches::cleanup() {
    if (!mln::caches::is_initialized()) {
        return;
    }

    s_db->delete_statement(s_saved_select_dump_channel);

    mln::caches::s_cluster = nullptr;
    mln::caches::s_db = nullptr;
}
bool mln::caches::is_initialized() {
    return mln::caches::s_cluster != nullptr && mln::caches::s_db != nullptr;
}
void mln::caches::init(dpp::cluster* cluster, database_handler* db) {
    mln::caches::s_cluster = cluster;
    mln::caches::s_db = db;

    if (!is_initialized()) {
        throw std::exception("Failed to initialize caches, either the cluster or the database are not valid references!");
    }

    //Note that some of the info in the events will not be filled if the cache policy of the cluster is ::cp_none (dpp does dpp::find_X for some stuff, which will be empty)
    s_cluster->on_guild_member_update([](const dpp::guild_member_update_t& event_data) { 
        const bool success = mln::caches::member_cache.update_element(std::make_tuple(event_data.updated.guild_id, event_data.updated.user_id), 
            event_data.updated).has_value();

        s_cluster->log(dpp::loglevel::ll_debug, success ? 
            "Updated guild member " + std::to_string(event_data.updated.user_id) + " from guild " + std::to_string(event_data.updated.guild_id) :
            "Not updated guild member " + std::to_string(event_data.updated.user_id) + " from guild " + std::to_string(event_data.updated.guild_id));
        });

    s_cluster->on_channel_update([](const dpp::channel_update_t& event_data) { 
        if (event_data.updated != nullptr) {
            const dpp::channel& channel = *event_data.updated;
            const bool success = mln::caches::channel_cache.update_element(event_data.updated->id, channel).has_value();

            s_cluster->log(dpp::loglevel::ll_debug, success ?
                "Updated channel " + std::to_string(channel.id) + " from guild " + std::to_string(channel.guild_id) :
                "Not updated channel " + std::to_string(channel.id) + " from guild " + std::to_string(channel.guild_id));
        }
        });

    s_cluster->on_user_update([](const dpp::user_update_t& event_data) { 
        const bool success = mln::caches::user_cache.update_element(event_data.updated.id, event_data.updated).has_value();

        s_cluster->log(dpp::loglevel::ll_debug, success ?
            "Updated user " + std::to_string(event_data.updated.id) :
            "Not updated user " + std::to_string(event_data.updated.id));
        });

    s_cluster->on_guild_update([](const dpp::guild_update_t& event_data) { 
        if (event_data.updated != nullptr) {
            const dpp::guild& guild = *event_data.updated;
            const bool success = mln::caches::guild_cache.update_element(event_data.updated->id, guild).has_value();

            s_cluster->log(dpp::loglevel::ll_debug, success ?
                "Updated guild " + std::to_string(guild.id) :
                "Not updated guild " + std::to_string(guild.id));
        }
        });

    s_cluster->on_guild_role_update([](const dpp::guild_role_update_t& event_data) { 
        if (event_data.updated != nullptr) {
            const dpp::role& role = *event_data.updated;
            const bool success = mln::caches::role_cache.update_element(event_data.updated->id, role).has_value();

            s_cluster->log(dpp::loglevel::ll_debug, success ?
                "Updated role " + std::to_string(role.id) :
                "Not updated role " + std::to_string(role.id));
        }
        });

    mln::db_result res = s_db->save_statement("SELECT dedicated_channel_id FROM guild_profile WHERE guild_id = ?1;", s_saved_select_dump_channel);
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while saving the select dump channel stmt: " + 
            mln::database_handler::get_name_from_result(res) + ". " + db->get_last_err_msg();
        throw std::exception(err_msg.c_str());
    }
}

std::atomic_ullong mln::caches::s_cache_misses{0};
std::atomic_ullong mln::caches::s_cache_requests{0};
dpp::cluster* mln::caches::s_cluster{nullptr};
mln::database_handler* mln::caches::s_db{nullptr};

mln::cache_primitive<uint64_t, uint64_t, 10000, 1000, 0.75, true> mln::caches::dump_channels_cache{};
mln::cache<uint64_t, std::vector<std::string>, false, 400, 30, 0.7, true> mln::caches::show_all_cache{};
mln::cache<std::tuple<uint64_t, uint64_t>, std::vector<std::string>, false, 1000, 100, 0.7, true, mln::caches::composite_tuple_hash> mln::caches::show_user_cache{};
mln::cache<uint64_t, dpp::guild, false, 3000, 300, 0.7, true> mln::caches::guild_cache{};
mln::cache<uint64_t, dpp::channel, false, 4000, 300, 0.7, true> mln::caches::channel_cache{};
mln::cache<uint64_t, dpp::user_identified, false, 6000, 500, 0.7, true> mln::caches::user_cache{};
mln::cache<std::tuple<uint64_t, uint64_t>, dpp::guild_member, false, 6000, 500, 0.7, true, mln::caches::composite_tuple_hash> mln::caches::member_cache{};
mln::cache<uint64_t, dpp::role, false, 6000, 500, 0.7, true> mln::caches::role_cache{};

std::optional<uint64_t> mln::caches::get_dump_channel_id(uint64_t guild_id) {
    if (guild_id == 0) {
        return std::nullopt;
    }

	++mln::caches::s_cache_requests;

    //Look in cache
	std::optional<uint64_t> result = mln::caches::dump_channels_cache.get_element(guild_id);
	if (result.has_value()) {
		return result;
	}

	++mln::caches::s_cache_misses;

    //Look in database
    db_result res = s_db->bind_parameter(s_saved_select_dump_channel, 0, 1, static_cast<int64_t>(guild_id));
    if (res != mln::db_result::ok) {
        s_cluster->log(dpp::loglevel::ll_error, "Failed to bind query parameters for select dump channel! Error: " + 
            mln::database_handler::get_name_from_result(res) + ", " + s_db->get_last_err_msg());
        return std::nullopt;
    }

    mln::database_callbacks_t calls{};
    calls.callback_data = static_cast<void*>(&result);
    calls.type_definer_callback = [](void*, int) {return false;};
    calls.data_adder_callback = [](void* d, int col, mln::db_column_data_t&& c_data) {
        std::optional<uint64_t>* opt_ptr = static_cast<std::optional<uint64_t>*>(d);

        *opt_ptr = static_cast<uint64_t>(std::get<int64_t>(c_data.data));
    };

    s_db->exec(s_saved_select_dump_channel, calls);
    if (mln::database_handler::is_exec_error(res)) {
        s_cluster->log(dpp::loglevel::ll_error, "Failed to execute query for select dump channel! Error: " + 
            mln::database_handler::get_name_from_result(res) + ", " + s_db->get_last_err_msg());
    }

    //If value found in database, store it in cache and return it
    if (result.has_value()) {
        return mln::caches::dump_channels_cache.add_element(guild_id, result.value());
    }   

	return std::nullopt;
}
std::optional<dpp::message> mln::caches::get_message(uint64_t message_id, const dpp::interaction_create_t* const event_data) {
    if (message_id == 0) {
        return std::nullopt;
    }

    //Look in resolved cache
    if (event_data != nullptr) {
        if (event_data->command.msg.id == message_id) {
            return event_data->command.msg;
        } else {

            const auto it = event_data->command.resolved.messages.find(message_id);
            if (it != event_data->command.resolved.messages.end()) {
                return it->second;
            }
        }
    }

    return std::nullopt;
}

dpp::task<std::optional<dpp::message>> mln::caches::get_message_task(uint64_t message_id, uint64_t channel_id) {
    if (message_id == 0) {
        co_return std::nullopt;
    }

    const dpp::confirmation_callback_t result = co_await s_cluster->co_message_get(message_id, channel_id);
    if (result.is_error()) {
        s_cluster->log(dpp::loglevel::ll_error, "Failed to retrieve msg from message_id " + std::to_string(message_id) +
            " from channel_id " + std::to_string(channel_id) + "! Error: " + result.get_error().human_readable);
        co_return std::nullopt;
    }

    co_return std::move(result.get<dpp::message>());
}

std::optional<std::shared_ptr<const std::vector<std::string>>> mln::caches::get_show_all(uint64_t guild_id) {
    if (guild_id == 0) {
        return std::nullopt;
    }

    ++mln::caches::s_cache_requests;

    //Look in cache
    std::optional<std::shared_ptr<const std::vector<std::string>>> result = mln::caches::show_all_cache.get_element(guild_id);
    if (!result.has_value()) {
        ++mln::caches::s_cache_misses;
    }

    return std::nullopt;
}
std::optional<std::shared_ptr<const std::vector<std::string>>> mln::caches::get_show_user(const std::tuple<uint64_t, uint64_t>& guild_user_ids) {
    if (std::get<0>(guild_user_ids) == 0 || std::get<1>(guild_user_ids) == 0) {
        return std::nullopt;
    }

    ++mln::caches::s_cache_requests;

    //Look in cache
    std::optional<std::shared_ptr<const std::vector<std::string>>> result = mln::caches::show_user_cache.get_element(guild_user_ids);
    if (!result.has_value()) {
        ++mln::caches::s_cache_misses;
    }

    return std::nullopt;
}
std::optional<std::shared_ptr<const dpp::guild>> mln::caches::get_guild(uint64_t guild_id) {
    if (guild_id == 0) {
        return std::nullopt;
    }

    ++mln::caches::s_cache_requests;

    //Look in cache
    std::optional<std::shared_ptr<const dpp::guild>> result = mln::caches::guild_cache.get_element(guild_id);
    if (result.has_value()) {
        return result;
    }

    ++mln::caches::s_cache_misses;

    return std::nullopt;
}
dpp::task<std::optional<std::shared_ptr<const dpp::guild>>> mln::caches::get_guild_task(uint64_t guild_id) {
    if (guild_id == 0) {
        co_return std::nullopt;
    }

    const dpp::confirmation_callback_t result = co_await s_cluster->co_guild_get(guild_id);
    if (result.is_error()) {
        s_cluster->log(dpp::loglevel::ll_error, "Failed to retrieve guild from guild_id " + std::to_string(guild_id) + 
            "! Error: " + result.get_error().human_readable);
        co_return std::nullopt;
    }

    co_return mln::caches::guild_cache.add_element(guild_id, std::move(result.get<dpp::guild>()));
}
std::optional<std::shared_ptr<const dpp::channel>> mln::caches::get_channel(uint64_t channel_id, const dpp::interaction_create_t* event_data) {
    if (channel_id == 0) {
        return std::nullopt;
    }

    ++mln::caches::s_cache_requests;

    //Look in cache
    std::optional<std::shared_ptr<const dpp::channel>> result = mln::caches::channel_cache.get_element(channel_id);
    if (result.has_value()) {
        return result;
    }

    ++mln::caches::s_cache_misses;

    //Look in resolved cache
    if (event_data != nullptr) {
        if (event_data->command.channel.id == channel_id) {
            return mln::caches::channel_cache.add_element(channel_id, event_data->command.channel);
        } else {

            const auto it = event_data->command.resolved.channels.find(channel_id);
            if (it != event_data->command.resolved.channels.end()) {
                return mln::caches::channel_cache.add_element(channel_id, it->second);
            }
        }
    }

    return std::nullopt;
}
dpp::task<std::optional<std::shared_ptr<const dpp::channel>>> mln::caches::get_channel_task(uint64_t channel_id) {
    if (channel_id == 0) {
        co_return std::nullopt;
    }

    const dpp::confirmation_callback_t result = co_await s_cluster->co_channel_get(channel_id);
    if (result.is_error()) {
        s_cluster->log(dpp::loglevel::ll_error, "Failed to retrieve channel from channel_id " + std::to_string(channel_id) +
            "! Error: " + result.get_error().human_readable);
        co_return std::nullopt;
    }

    co_return mln::caches::channel_cache.add_element(channel_id, std::move(result.get<dpp::channel>()));
}
std::optional<std::shared_ptr<const dpp::user_identified>> mln::caches::get_user(uint64_t user_id, const dpp::interaction_create_t* event_data) {
    if (user_id == 0) {
        return std::nullopt;
    }

    ++mln::caches::s_cache_requests;

    //Look in cache
    std::optional<std::shared_ptr<const dpp::user_identified>> result = mln::caches::user_cache.get_element(user_id);
    if (result.has_value()) {
        return result;
    }

    ++mln::caches::s_cache_misses;

    //Look in resolved cache
    if (event_data != nullptr) {
        if (event_data->command.usr.id == user_id) {
            return mln::caches::user_cache.add_element(user_id, event_data->command.usr);
        } else {

            const auto it = event_data->command.resolved.users.find(user_id);
            if (it != event_data->command.resolved.users.end()) {
                return mln::caches::user_cache.add_element(user_id, it->second);
            }
        }
    }

    return std::nullopt;
}
dpp::task<std::optional<std::shared_ptr<const dpp::user_identified>>> mln::caches::get_user_task(uint64_t user_id) {
    if (user_id == 0) {
        co_return std::nullopt;
    }

    const dpp::confirmation_callback_t result = co_await s_cluster->co_user_get(user_id);
    if (result.is_error()) {
        s_cluster->log(dpp::loglevel::ll_error, "Failed to retrieve user from user_id " + std::to_string(user_id) +
            "! Error: " + result.get_error().human_readable);
        co_return std::nullopt;
    }

    co_return mln::caches::user_cache.add_element(user_id, result.get<dpp::user_identified>());
}
std::optional<std::shared_ptr<const dpp::guild_member>> mln::caches::get_member(const std::tuple<uint64_t, uint64_t>& guild_user_ids,
    const dpp::interaction_create_t* event_data) {
    if (std::get<0>(guild_user_ids) == 0 || std::get<1>(guild_user_ids) == 0) {
        return std::nullopt;
    }

    ++mln::caches::s_cache_requests;

    //Look in cache
    std::optional<std::shared_ptr<const dpp::guild_member>> result = mln::caches::member_cache.get_element(guild_user_ids);
    if (result.has_value()) {
        return result;
    }

    ++mln::caches::s_cache_misses;

    //Look in resolved cache
    if (event_data != nullptr) {
        if (event_data->command.member.guild_id == std::get<0>(guild_user_ids) && event_data->command.member.user_id == std::get<1>(guild_user_ids)) {
            return mln::caches::member_cache.add_element(guild_user_ids, event_data->command.member);
        } else {

            const auto it = event_data->command.resolved.members.find(std::get<1>(guild_user_ids));
            if (it != event_data->command.resolved.members.end()) {
                return mln::caches::member_cache.add_element(guild_user_ids, it->second);
            }
        }
    }

    return std::nullopt;
}
dpp::task<std::optional<std::shared_ptr<const dpp::guild_member>>> mln::caches::get_member_task(const std::tuple<uint64_t, uint64_t>& guild_user_ids) {
    if (std::get<0>(guild_user_ids) == 0 || std::get<1>(guild_user_ids) == 0) {
        co_return std::nullopt;
    }

    const dpp::confirmation_callback_t result = co_await s_cluster->co_guild_get_member(std::get<0>(guild_user_ids), std::get<1>(guild_user_ids));
    if (result.is_error()) {
        s_cluster->log(dpp::loglevel::ll_error, "Failed to retrieve member from guild_id " + std::to_string(std::get<0>(guild_user_ids)) +
            " from user_id " + std::to_string(std::get<1>(guild_user_ids)) + "! Error: " + result.get_error().human_readable);
        co_return std::nullopt;
    }

    co_return mln::caches::member_cache.add_element(guild_user_ids, std::move(result.get<dpp::guild_member>()));
}
std::optional<std::shared_ptr<const dpp::role>> mln::caches::get_role(uint64_t role_id, const dpp::interaction_create_t* event_data) {
    if (role_id == 0) {
        return std::nullopt;
    }

    ++mln::caches::s_cache_requests;

    //Look in cache
    std::optional<std::shared_ptr<const dpp::role>> result = mln::caches::role_cache.get_element(role_id);
    if (result.has_value()) {
        return result;
    }

    ++mln::caches::s_cache_misses;

    //Look in resolved cache
    if (event_data != nullptr) {
        const auto it = event_data->command.resolved.roles.find(role_id);
        if (it != event_data->command.resolved.roles.end()) {     
            return mln::caches::role_cache.add_element(role_id, it->second);
        }
    }

    return std::nullopt;
}
dpp::task<std::optional<std::shared_ptr<const dpp::role>>> mln::caches::get_role_task(uint64_t guild_id, uint64_t role_id, bool add_all_guild_roles) {
    if (role_id == 0 || guild_id == 0) {
        co_return std::nullopt;
    }

    const dpp::confirmation_callback_t result = co_await s_cluster->co_roles_get(guild_id);
    if (result.is_error()) {
        s_cluster->log(dpp::loglevel::ll_error, "Failed to retrieve role from guild_id " + std::to_string(guild_id) +
            " from role_id " + std::to_string(role_id) + "! Error: " + result.get_error().human_readable);
        co_return std::nullopt;
    }

    dpp::role_map map = result.get<dpp::role_map>();
    //Add all the roles found if requested, otherwise add only the requested role
    if (add_all_guild_roles) {
        for (const std::pair<dpp::snowflake, dpp::role>& role_pair : map) {
            mln::caches::role_cache.add_element(role_pair.first, role_pair.second);
        }

        co_return mln::caches::role_cache.get_element(role_id);
    }

    const auto it = map.find(role_id);
    if (it == map.end()) {
        co_return std::nullopt;
    }

    co_return mln::caches::role_cache.add_element(role_id, it->second);;
}