#include "commands/slash/db/db_select.h"
#include "database/database_handler.h"
#include "bot_delta.h"
#include "utility/utility.h"

#include <regex>

mln::db_select::db_select(bot_delta* const delta) : base_db_command(delta),
saved_stmt(), saved_verbose_stmt(), saved_param_guild(), saved_param_name(), saved_param_verbose_guild(), saved_param_verbose_name(), valid_stmt(true) {

    mln::db_result res1 = delta->db.save_statement("SELECT * FROM storage WHERE guild_id = :GGG AND name = :NNN;", saved_verbose_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save select verbose stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    } else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(saved_verbose_stmt, 0, ":GGG", saved_param_verbose_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(saved_verbose_stmt, 0, ":NNN", saved_param_verbose_name);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save select verbose stmt param indexes!");
            valid_stmt = false;
        }
    }

    res1 = delta->db.save_statement("SELECT url, url_type, desc FROM storage WHERE guild_id = :GGG AND name = :NNN;", saved_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save select stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    } else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(saved_stmt, 0, ":GGG", saved_param_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(saved_stmt, 0, ":NNN", saved_param_name);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save select stmt param indexes!");
            valid_stmt = false;
        }
    }
}

dpp::task<void> mln::db_select::command(const dpp::command_data_option&, const dpp::slashcommand_t& event_data, url_type) {
    const dpp::command_value broadcast_param = event_data.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(!broadcast);

    if (!valid_stmt) {
        co_await thinking;
        event_data.edit_response("Failed database operation, the database was not initialized correctly!");
        co_return;
    }

    const dpp::command_value verbose_param = event_data.get_parameter("verbose");
    const bool verbose = std::holds_alternative<bool>(verbose_param) ? std::get<bool>(verbose_param) : false;

    const std::string name = std::get<std::string>(event_data.get_parameter("name"));

    mln::db_result res1, res2;
    if (verbose) {
        res1 = delta()->db.bind_parameter(saved_verbose_stmt, 0, saved_param_verbose_guild, static_cast<int64_t>(event_data.command.guild_id));
        res2 = delta()->db.bind_parameter(saved_verbose_stmt, 0, saved_param_verbose_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
    } else {
        res1 = delta()->db.bind_parameter(saved_stmt, 0, saved_param_guild, static_cast<int64_t>(event_data.command.guild_id));
        res2 = delta()->db.bind_parameter(saved_stmt, 0, saved_param_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
    }

    if (res1 != mln::db_result::ok || res2 != mln::db_result::ok) {
        co_await thinking;
        event_data.edit_response("Failed to bind query params, internal error!");
        co_return;
    }

    mln::db_select::record_data_t data{.verbose = verbose, .found = false};
    mln::database_callbacks_t callbacks{};
    callbacks.callback_data = static_cast<void*>(&data);
    callbacks.data_adder_callback = [](void* d, int c, mln::db_column_data_t&& c_d) {
        mln::db_select::record_data_t* data_ptr = static_cast<mln::db_select::record_data_t*>(d);
        data_ptr->found = true;//TODO make a state machine instead of this mess. At start I use starting func, then every step I switch to func for next column. No if statements required (other than checking for NULL)
        if (!data_ptr->verbose) {
            if (c == 0) {
                data_ptr->url = std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(c_d.data)));
            } else if (c == 1) {
                data_ptr->type = static_cast<mln::url_type>(std::get<int>(c_d.data));
                data_ptr->url_type = mln::utility::get_string(data_ptr->type);
            } else if (std::holds_alternative<const unsigned char*>(c_d.data)) {
                data_ptr->desc = std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(c_d.data)));
            }
            return;
        }

        switch (c) {
            case 0:
                break;
            case 1:
                break;
            case 2:
                data_ptr->url = std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(c_d.data)));
                break;
            case 3:
                data_ptr->type = static_cast<mln::url_type>(std::get<int>(c_d.data));
                data_ptr->url_type = mln::utility::get_string(data_ptr->type);
                break;
            case 4:
                data_ptr->desc = std::holds_alternative<const unsigned char*>(c_d.data) ? std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(c_d.data))) : "NULL";
                break;
            case 5:
                data_ptr->usr = std::to_string(static_cast<uint64_t>(std::get<int64_t>(c_d.data)));
                break;
            case 6:
                data_ptr->time = std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(c_d.data)));
                break;
            default:
                break;
        }
    };
    callbacks.type_definer_callback = [](void* d, int c) {return (static_cast<mln::db_select::record_data_t*>(d)->verbose ? c != 0 && c != 5 : true); };

    dpp::message msg{};
    if (!broadcast) {
        msg.set_flags(dpp::m_ephemeral);
    }
    mln::db_result res = delta()->db.exec(verbose ? saved_verbose_stmt : saved_stmt, callbacks);
    if (res != mln::db_result::ok || !data.found) {
        msg.set_content(res == mln::db_result::ok && !data.found ? "Failed to select record, no record found!" : "Failed to select record, internal error!");
    } else {
        if (data.type == mln::url_type::file) {
            msg.set_content(verbose ? ("name: " + name + ", url: " + data.url + ", type: [" + data.url_type + "], desc: " + (data.desc.empty() ? "NULL" : data.desc) + ", user: " + data.usr + ", creation: " + data.time) : data.url + ", [" + data.url_type + "]" + (data.desc.empty() ? "" : ", " + data.desc));
        } else {

            if (data.url.empty()) {
                co_await thinking;
                event_data.edit_response("Failed to retrieve original text message!");
                co_return;
            }
            std::regex url_regex(R"(https://discord.com/channels/(\d+)/(\d+)/(\d+))");
            std::smatch match;

            const bool regex_result = std::regex_search(data.url, match, url_regex);
            if (!regex_result || match.size() != 4) {
                co_await thinking;
                event_data.edit_response("Failed to retrieve message and channel ids from the url!");
                co_return;
            }

            const dpp::snowflake channel_id = std::stoull(match[2].str());
            const dpp::snowflake message_id = std::stoull(match[3].str());

            const dpp::confirmation_callback_t retrieved_result = co_await delta()->bot.co_messages_get(channel_id, message_id, dpp::snowflake{0}, dpp::snowflake{0}, 1);

            if (retrieved_result.is_error()) {
                co_await thinking;
                event_data.edit_response("Failed to retrieve original text message! Error: " + retrieved_result.get_error().human_readable);
                co_return;
            }

            const dpp::message_map original_text_map = retrieved_result.get<dpp::message_map>();
            const auto& it = original_text_map.find(message_id);

            if (it == original_text_map.end()) {
                co_await thinking;
                event_data.edit_response("Failed to retrieve original text message! The original text was not found.");
                co_return;
            }

            const dpp::message original_text_msg = it->second;

            if (original_text_msg.embeds.size() == 0) {
                co_await thinking;
                event_data.edit_response("Failed to retrieve original text message! The original text does not have an embed to retrieve the text from.");
                co_return;
            }

            dpp::embed embed{};
            embed.description = original_text_msg.embeds[0].description;
            msg.add_embed(embed);
            
            msg.set_content(verbose ? ("name: " + name + ", type: [" + data.url_type + "], desc: " + (data.desc.empty() ? "NULL" : data.desc) + ", user: " + data.usr + ", creation: " + data.time) : name + (data.desc.empty() ? "" : ", " + data.desc));
        }
    }

    co_await thinking;
    event_data.edit_response(msg);
}
