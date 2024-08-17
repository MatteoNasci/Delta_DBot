#include "commands/slash/db/db_show_records.h"
#include "database/database_handler.h"
#include "bot_delta.h"
#include "utility/utility.h"

mln::db_show_records::db_show_records(bot_delta* const delta) : base_db_command(delta),
saved_stmt(), saved_verbose_stmt(), valid_stmt(true) {

    mln::db_result res1 = delta->db.save_statement("SELECT * FROM storage WHERE guild_id = ?;", saved_verbose_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save select (show records) verbose stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }

    res1 = delta->db.save_statement("SELECT name, url_type, desc FROM storage WHERE guild_id = ?;", saved_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save select (show records) stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }
}

dpp::task<void> mln::db_show_records::command(const dpp::command_data_option&, const dpp::slashcommand_t& event_data, url_type) {
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

    mln::db_result res = verbose ? delta()->db.bind_parameter(saved_verbose_stmt, 0, 1, static_cast<int64_t>(event_data.command.guild_id)) : delta()->db.bind_parameter(saved_stmt, 0, 1, static_cast<int64_t>(event_data.command.guild_id));

    if (res != mln::db_result::ok) {
        co_await thinking;
        event_data.edit_response("Failed to bind query params, internal error!");
        co_return;
    }

    mln::database_callbacks_t callbacks{};
    size_t current_index = 0;
    std::vector<std::tuple<std::string, std::string, std::string>> n_v_list;
    std::vector<std::tuple<std::string, std::string, std::string , std::string, std::string>> v_list;
    callbacks.data_adder_callback = [verbose, &n_v_list, &v_list, &current_index](void*, int c, mln::db_column_data_t&& d) {
        if (!verbose) {
            if (c == 0) {//TODO make a state machine instead of this mess. At start I use starting func, then every step I switch to func for next column. No if statements required (other than checking for NULL)
                current_index = n_v_list.size();
                n_v_list.push_back(std::make_tuple(std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data))), std::string(), std::string()));
            } else if (c == 1) {
                std::get<1>(n_v_list[current_index]) = mln::utility::get_string(static_cast<mln::url_type>(std::get<int>(d.data)));
            } else {
                if (std::holds_alternative<const unsigned char*>(d.data)) {
                    std::get<2>(n_v_list[current_index]) = std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data)));
                }
            }
            return;
        }

        switch (c) {
            case 0:
                break;
            case 1:
                current_index = v_list.size();
                v_list.push_back(std::make_tuple(std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data))), std::string(), std::string(), std::string(), std::string()));
                break;
            case 2:
                break;
            case 3:
                std::get<1>(v_list[current_index]) = mln::utility::get_string(static_cast<mln::url_type>(std::get<int>(d.data)));
                break;
            case 4:
                std::get<2>(v_list[current_index]) = std::holds_alternative<const unsigned char*>(d.data) ? std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data))) : "NULL";
                break;
            case 5:
                std::get<3>(v_list[current_index]) = std::to_string(static_cast<uint64_t>(std::get<int64_t>(d.data)));
                break;
            case 6:
                std::get<4>(v_list[current_index]) = std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data)));
                break;
            default:
                break;
        }
    };
    callbacks.type_definer_callback = [verbose](void*, int c) {return (verbose ? c != 0 && c != 5 : true); };

    dpp::message msg{};
    if (!broadcast) {
        msg.set_flags(dpp::m_ephemeral);
    }

    res = delta()->db.exec(verbose ? saved_verbose_stmt : saved_stmt, callbacks);
    if (res != mln::db_result::ok || (n_v_list.empty() && v_list.empty())) {
        msg.set_content(res == mln::db_result::ok && (n_v_list.empty() && v_list.empty()) ? "Failed to show_records, empty database!" : "Failed to show_records, internal error!");
        co_await thinking;
        event_data.edit_response(msg);
    } else {
        const std::function<std::string(size_t, size_t, size_t)> v_func = [&v_list](size_t current_index, size_t, size_t) {
            if (current_index >= v_list.size()) {
                return std::string{};
            }
            const std::tuple<std::string, std::string, std::string, std::string, std::string>& tup = v_list[current_index];
            return "{ name: " + std::get<0>(tup) + ", type: [" + std::get<1>(tup) + "], description: " + std::get<2>(tup) + ", usr: " + std::get<3>(tup) + ", time: " + std::get<4>(tup) + " }\n";
        };
        const std::function<std::string(size_t, size_t, size_t)> n_v_func = [&n_v_list](size_t current_index, size_t, size_t) {
            if (current_index >= n_v_list.size()) {
                return std::string{};
            }
            const std::tuple<std::string, std::string, std::string>& tup = n_v_list[current_index];
            return "{ " + std::get<0>(tup) + ", type: [" + std::get<1>(tup) + "]" + (std::get<2>(tup).empty() ? " }\n" : ", description: " + std::get<2>(tup) + " }\n");
        };

        co_await thinking;
        bool result = co_await mln::utility::send_msg_recursively_embed(delta()->bot, event_data, verbose ? v_func : n_v_func, event_data.command.usr.id, true, broadcast);
        if (!result) {
            delta()->bot.log(dpp::loglevel::ll_error, "An error occurred while sending embeds recursively for /db op show_records command!");
        }
    }
}
