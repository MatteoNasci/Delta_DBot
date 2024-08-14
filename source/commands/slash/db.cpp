#include "commands/slash/db.h"
#include "bot_delta.h"
#include "utility/constants.h"
#include "utility/utility.h"

#include <dpp/queues.h>

mln::db::db(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("db", "Manage the database.", delta->bot.me.id) //TODO add verbose boolean option to select commands, when false only show minimal info/only file, when true show all I can show
        .add_option(dpp::command_option(dpp::co_sub_command_group, "op", "Perform an operation on the db", false)
            .add_option(dpp::command_option(dpp::co_sub_command, "insert", "Inserts a new record in the db. It will fail if the given name is not unique!", false)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the file.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_string, "description", "Small description of the stored file. Default: NULL", false)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))))
            .add_option(dpp::command_option(dpp::co_sub_command, "insert_replace", "Inserts or replaces a record in the db.", false)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the file.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_string, "description", "Small description of the stored file. Default: NULL", false)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))))
            .add_option(dpp::command_option(dpp::co_sub_command, "select", "Selects a record from the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the file.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "show_records", "Shows all the db records names.", false)
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "update", "Updates an existing record in the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the file.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_string, "description", "Small description of the stored file. Default: NULL", false)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))))
            .add_option(dpp::command_option(dpp::co_sub_command, "update_description", "Updates an existing record description in the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the file.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_string, "description", "Small description of the stored file. Default: NULL", false)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))))
            .add_option(dpp::command_option(dpp::co_sub_command, "remove", "Removes an existing record in the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the file.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))))) , 
    saved_insert_stmt(), saved_insert_replace_stmt(), saved_select_stmt(), saved_show_records_stmt(), saved_update_stmt(), saved_update_desc_stmt(), saved_remove_stmt(),
    saved_insert_guild(), saved_insert_user(), saved_insert_file_name(), saved_insert_file_url(), saved_insert_desc(),
    saved_insert_replace_guild(), saved_insert_replace_user(), saved_insert_replace_file_name(), saved_insert_replace_file_url(), saved_insert_replace_desc(),
    saved_select_guild(), saved_select_file_name(),
    saved_update_guild(), saved_update_user(), saved_update_file_name(), saved_update_file_url(), saved_update_desc(),
    saved_update_desc_guild(), saved_update_desc_user(), saved_update_desc_file_name(), saved_update_desc_desc(),
    saved_remove_guild(), saved_remove_user(), saved_remove_file_name(), valid_stmt(true) {
    
    auto res1 = delta->db.save_statement("INSERT OR ABORT INTO file (guild_id, file_name, file_url, file_desc, user_id) VALUES(:GGG, :NNN, :FFF, :DDD, :UUU);", saved_insert_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }else {
        auto res11 = delta->db.get_bind_parameter_index(saved_insert_stmt, 0, ":GGG", saved_insert_guild);
        auto res12 = delta->db.get_bind_parameter_index(saved_insert_stmt, 0, ":NNN", saved_insert_file_name);
        auto res13 = delta->db.get_bind_parameter_index(saved_insert_stmt, 0, ":FFF", saved_insert_file_url);
        auto res14 = delta->db.get_bind_parameter_index(saved_insert_stmt, 0, ":UUU", saved_insert_user);
        auto res15 = delta->db.get_bind_parameter_index(saved_insert_stmt, 0, ":DDD", saved_insert_desc);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok || res14 != mln::db_result::ok || res15 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert stmt param indexes!");
            valid_stmt = false;
        }
    }

    res1 = delta->db.save_statement("INSERT OR REPLACE INTO file (guild_id, file_name, file_url, file_desc, user_id) VALUES(:GGG, :NNN, :FFF, :DDD, :UUU);", saved_insert_replace_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert_replace stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }
    else {
        auto res11 = delta->db.get_bind_parameter_index(saved_insert_replace_stmt, 0, ":GGG", saved_insert_replace_guild);
        auto res12 = delta->db.get_bind_parameter_index(saved_insert_replace_stmt, 0, ":NNN", saved_insert_replace_file_name);
        auto res13 = delta->db.get_bind_parameter_index(saved_insert_replace_stmt, 0, ":FFF", saved_insert_replace_file_url);
        auto res14 = delta->db.get_bind_parameter_index(saved_insert_replace_stmt, 0, ":UUU", saved_insert_replace_user);
        auto res15 = delta->db.get_bind_parameter_index(saved_insert_replace_stmt, 0, ":DDD", saved_insert_replace_desc);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok || res14 != mln::db_result::ok || res15 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert_replace stmt param indexes!");
            valid_stmt = false;
        }
    }

    res1 = delta->db.save_statement("SELECT file_url, file_desc, user_id FROM file WHERE guild_id = :GGG AND file_name = :NNN;", saved_select_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save select stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }
    else {
        auto res11 = delta->db.get_bind_parameter_index(saved_select_stmt, 0, ":GGG", saved_select_guild);
        auto res12 = delta->db.get_bind_parameter_index(saved_select_stmt, 0, ":NNN", saved_select_file_name);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save select stmt param indexes!");
            valid_stmt = false;
        }
    }

    res1 = delta->db.save_statement("SELECT file_name, file_desc, user_id FROM file WHERE guild_id = ?;", saved_show_records_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save select (show records) stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }

    res1 = delta->db.save_statement("UPDATE OR ABORT file SET file_url = :FFF, file_desc = :DDD WHERE guild_id = :GGG AND file_name = :NNN AND user_id = :UUU;", saved_update_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save update stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }
    else {
        auto res11 = delta->db.get_bind_parameter_index(saved_update_stmt, 0, ":GGG", saved_update_guild);
        auto res12 = delta->db.get_bind_parameter_index(saved_update_stmt, 0, ":NNN", saved_update_file_name);
        auto res13 = delta->db.get_bind_parameter_index(saved_update_stmt, 0, ":FFF", saved_update_file_url);
        auto res14 = delta->db.get_bind_parameter_index(saved_update_stmt, 0, ":UUU", saved_update_user);
        auto res15 = delta->db.get_bind_parameter_index(saved_update_stmt, 0, ":DDD", saved_update_desc);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok || res14 != mln::db_result::ok || res15 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save update stmt param indexes!");
            valid_stmt = false;
        }
    }

    res1 = delta->db.save_statement("UPDATE OR ABORT file SET file_desc = :DDD WHERE guild_id = :GGG AND file_name = :NNN AND user_id = :UUU;", saved_update_desc_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save update_desc stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }
    else {
        auto res11 = delta->db.get_bind_parameter_index(saved_update_desc_stmt, 0, ":GGG", saved_update_desc_guild);
        auto res12 = delta->db.get_bind_parameter_index(saved_update_desc_stmt, 0, ":NNN", saved_update_desc_file_name);
        auto res13 = delta->db.get_bind_parameter_index(saved_update_desc_stmt, 0, ":UUU", saved_update_desc_user);
        auto res14 = delta->db.get_bind_parameter_index(saved_update_desc_stmt, 0, ":DDD", saved_update_desc_desc);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok || res14 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save update_desc stmt param indexes!");
            valid_stmt = false;
        }
    }

    res1 = delta->db.save_statement("DELETE FROM file WHERE guild_id = :GGG AND file_name = :NNN AND user_id = :UUU;", saved_remove_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save delete stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }
    else {
        auto res11 = delta->db.get_bind_parameter_index(saved_remove_stmt, 0, ":GGG", saved_remove_guild);
        auto res12 = delta->db.get_bind_parameter_index(saved_remove_stmt, 0, ":NNN", saved_remove_file_name);
        auto res13 = delta->db.get_bind_parameter_index(saved_remove_stmt, 0, ":UUU", saved_remove_user);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save delete stmt param indexes!");
            valid_stmt = false;
        }
    }
}
dpp::job mln::db::command(dpp::slashcommand_t event){//TODO put each sub command in its own .cpp file, too much shit here already (callbacks inside a .cpp interpreter as well)
    typedef std::function<dpp::task<void>(dpp::command_data_option&, const dpp::slashcommand_t&)> op_callback_f;
    static const std::unordered_map<std::string, op_callback_f> allowed_op_sub_commands{
        {"insert", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            auto waiting = event.co_thinking(false);

            std::string desc;
            const dpp::command_value desc_param = event.get_parameter("description");
            const bool valid_desc = std::holds_alternative<std::string>(desc_param);
            if (valid_desc) {
                desc = std::get<std::string>(desc_param);
            }

            int64_t guild_id = event.command.guild_id;
            int64_t user_id = event.command.usr.id;

            const dpp::snowflake file_id = std::get<dpp::snowflake>(event.get_parameter("file"));
            const dpp::attachment att = event.command.get_resolved_attachment(file_id); 

            auto r = co_await delta()->bot.co_request(att.url, dpp::http_method::m_get, "", att.content_type);

            dpp::message message = dpp::message("File uploaded:").add_file(att.filename, r.body, att.content_type).set_channel_id(event.command.channel_id).set_guild_id(event.command.guild_id);
            co_await waiting;
            co_await event.co_edit_response(message);
            auto co_mess = co_await event.co_get_original_response();

            if (co_mess.is_error()) {
                delta()->bot.log(dpp::loglevel::ll_debug, "Err insert: " + co_mess.get_error().human_readable);
                event.co_edit_response("Failed to insert element to database! Error: " + co_mess.get_error().human_readable);
                co_return;
            }
            dpp::message temp_msg = co_mess.get<dpp::message>();
            
            delta()->bot.log(dpp::loglevel::ll_debug, std::to_string(temp_msg.attachments.size()));
            delta()->bot.log(dpp::loglevel::ll_debug, temp_msg.attachments[0].url);
            std::string url = temp_msg.attachments[0].url;
            std::string name = std::get<std::string>(event.get_parameter("name"));
            //TODO dpp::job should not be used i think, task<void> to use. Study of coro works
            /*delta()->bot.message_delete(temp_msg.id, temp_msg.channel_id);*/
            //TODO deleting the message removes the url. I need an apposite channel to store images (save to guild_profile). If not set just print image in current channel and leave it there
            
            //TODO I need to "store" the original url to a dedicated channel to make the url work for a long time
            auto res1 = delta()->db.bind_parameter(saved_insert_stmt, 0, saved_insert_guild, guild_id);
            auto res2 = delta()->db.bind_parameter(saved_insert_stmt, 0, saved_insert_user, user_id);
            auto res3 = delta()->db.bind_parameter(saved_insert_stmt, 0, saved_insert_file_url, url.c_str(), url.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            auto res4 = delta()->db.bind_parameter(saved_insert_stmt, 0, saved_insert_file_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            mln::db_result res5;
            if (valid_desc) {
                res5 = delta()->db.bind_parameter(saved_insert_stmt, 0, saved_insert_desc, desc.c_str(), desc.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            }
            else {
                res5 = delta()->db.bind_parameter(saved_insert_stmt, 0, saved_insert_desc);
            }

            if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok || res4 != mln::db_result::ok || res5 != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind insert params!" + name + " " + desc);
                temp_msg.set_content("Failed to insert element, internal error!");
                event.edit_original_response(temp_msg);
                co_return;
            }

            auto res = delta()->db.exec(saved_insert_stmt, mln::database_callbacks_t());
            if (res != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to insert element!");
                temp_msg.set_content("Failed to insert element, internal error!");
            }else {
                temp_msg.set_content("Element inserted to the db!");
            }

            event.edit_original_response(temp_msg);
        }},
        {"insert_replace", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            auto waiting = event.co_thinking(false);//this one requires !broadcast since the condition wants true for ephemeral

            std::string desc;
            const dpp::command_value desc_param = event.get_parameter("description");
            const bool valid_desc = std::holds_alternative<std::string>(desc_param);
            if (valid_desc) {
                desc = std::get<std::string>(desc_param);
            }

            int64_t guild_id = event.command.guild_id;
            int64_t user_id = event.command.usr.id;

            const dpp::snowflake file_id = std::get<dpp::snowflake>(event.get_parameter("file"));
            const dpp::attachment att = event.command.get_resolved_attachment(file_id);

            auto r = co_await delta()->bot.co_request(att.url, dpp::http_method::m_get, "", att.content_type);

            dpp::message message = dpp::message("File uploaded:").add_file(att.filename, r.body, att.content_type).set_channel_id(event.command.channel_id).set_guild_id(event.command.guild_id);
            co_await waiting;
            co_await event.co_edit_response(message);
            auto co_mess = co_await event.co_get_original_response();

            if (co_mess.is_error()) {
                delta()->bot.log(dpp::loglevel::ll_debug, "Err insert or replace: " + co_mess.get_error().human_readable);
                event.co_edit_response("Failed to insert or replace element to database! Error: " + co_mess.get_error().human_readable);
                co_return;
            }
            dpp::message temp_msg = co_mess.get<dpp::message>();

            delta()->bot.log(dpp::loglevel::ll_debug, std::to_string(temp_msg.attachments.size()));
            delta()->bot.log(dpp::loglevel::ll_debug, temp_msg.attachments[0].url);
            std::string url = temp_msg.attachments[0].url;
            std::string name = std::get<std::string>(event.get_parameter("name"));

            //TODO I might need to "store" the original url to a dedicated channel to make the url work
            auto res1 = delta()->db.bind_parameter(saved_insert_replace_stmt, 0, saved_insert_replace_guild, guild_id);
            auto res2 = delta()->db.bind_parameter(saved_insert_replace_stmt, 0, saved_insert_replace_user, user_id);
            auto res3 = delta()->db.bind_parameter(saved_insert_replace_stmt, 0, saved_insert_replace_file_url, url.c_str(), url.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            auto res4 = delta()->db.bind_parameter(saved_insert_replace_stmt, 0, saved_insert_replace_file_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            mln::db_result res5;
            if (valid_desc) {
                res5 = delta()->db.bind_parameter(saved_insert_replace_stmt, 0, saved_insert_replace_desc, desc.c_str(), desc.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            }
            else {
                res5 = delta()->db.bind_parameter(saved_insert_replace_stmt, 0, saved_insert_replace_desc);
            }

            if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok || res4 != mln::db_result::ok || res5 != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind insert_replace params!" + name + " " + desc);
                temp_msg.set_content("Failed to insert_replace element, internal error!");
                event.edit_original_response(temp_msg);
                co_return;
            }

            auto res = delta()->db.exec(saved_insert_replace_stmt, mln::database_callbacks_t());
            if (res != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to insert_replace element!");
                temp_msg.set_content("Failed to insert_replace element, internal error!");
            }else {
                temp_msg.set_content("Element inserted_replaced to the db!");
            }

            event.edit_original_response(temp_msg);
        }},
        {"select", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            const dpp::command_value broadcast_param = event.get_parameter("broadcast");
            const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
            auto waiting = event.co_thinking(!broadcast);//this one requires !broadcast since the condition wants true for ephemeral

            std::string name = std::get<std::string>(event.get_parameter("name"));

            auto res1 = delta()->db.bind_parameter(saved_select_stmt, 0, saved_select_guild, static_cast<int64_t>(event.command.guild_id));
            auto res2 = delta()->db.bind_parameter(saved_select_stmt, 0, saved_select_file_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            dpp::message msg{};
            if (!broadcast) {
                msg.set_flags(dpp::m_ephemeral);
            }

            if (res1 != mln::db_result::ok || res2 != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind select params!");
                msg.set_content("Failed to select, internal error!");
                co_await waiting;
                event.edit_response(msg);
                co_return;
            }

            mln::database_callbacks_t callbacks;
            std::string s{};
            callbacks.callback_data = &s;
            callbacks.data_adder_callback = [](void* s_v, int c, mln::db_column_data_t&& d) {
                std::string* s_p = static_cast<std::string*>(s_v);
                if (c == 0) {
                    *s_p += "{ " + std::string(d.name) + " : " + std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data)));
                }else if(c == 1) {
                    *s_p += " | " + std::string(d.name) + " : " + (std::holds_alternative<const short*>(d.data) ? "NULL" : std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data))));
                }else {
                    *s_p += " | " + std::string(d.name) + " : " + std::to_string(static_cast<uint64_t>(std::get<int64_t>(d.data))) + " }\n";
                }
            };
            callbacks.row_callback = nullptr;
            callbacks.statement_index_callback = nullptr;
            callbacks.type_definer_callback = [](void*, int c) {return c <= 1; };

            auto res = delta()->db.exec(saved_select_stmt, callbacks);
            if (res != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to select!");
                msg.set_content("Failed to select, internal error!");
            }
            else {
                msg.set_content(s);
                //TODO add elements to the reply, if text exceeds msg limit act depending on broadcast
            }

            co_await waiting;
            event.edit_response(msg);
        }},
        {"show_records", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            const dpp::command_value broadcast_param = event.get_parameter("broadcast");
            const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
            auto waiting = event.co_thinking(!broadcast);//this one requires !broadcast since the condition wants true for ephemeral

            auto res = delta()->db.bind_parameter(saved_show_records_stmt, 0, 1, static_cast<int64_t>(event.command.guild_id));
            dpp::message msg{};
            if (!broadcast) {
                msg.set_flags(dpp::m_ephemeral);
            }

            if (res != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind show_records params!");
                msg.set_content("Failed to show_records, internal error!");
                co_await waiting;
                event.edit_response(msg);
                co_return;
            }

            mln::database_callbacks_t callbacks;
            std::string s{};
            callbacks.callback_data = &s;
            callbacks.data_adder_callback = [](void* s_v, int c, mln::db_column_data_t&& d) {
                std::string* s_p = static_cast<std::string*>(s_v);
                if (c == 0) {
                    *s_p += "{ " + std::string(d.name) + " : " + std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data)));
                }else if (c == 1) {
                    *s_p += " | " + std::string(d.name) + " : " + (std::holds_alternative<const short*>(d.data) ? "NULL" : std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data))));
                }else {
                    *s_p += " | " + std::string(d.name) + " : " + std::to_string(static_cast<uint64_t>(std::get<int64_t>(d.data))) + " }\n";
                }
            };
            callbacks.row_callback = nullptr;
            callbacks.statement_index_callback = nullptr;
            callbacks.type_definer_callback = [](void*, int c) {return c <= 1; };

            res = delta()->db.exec(saved_show_records_stmt, callbacks);
            if (res != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to show_records!");
                msg.set_content("Failed to show_records, internal error!");
                co_await waiting;
                event.edit_response(msg);
            }
            else {
                co_await waiting;
                co_await mln::utility::send_msg_recursively(delta()->bot, event, s, event.command.usr.id, true, broadcast);
            }
        }},
        {"update", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            auto waiting = event.co_thinking(false);//this one requires !broadcast since the condition wants true for ephemeral

            std::string desc;
            const dpp::command_value desc_param = event.get_parameter("description");
            const bool valid_desc = std::holds_alternative<std::string>(desc_param);
            if (valid_desc) {
                desc = std::get<std::string>(desc_param);
            }

            int64_t guild_id = event.command.guild_id;
            int64_t user_id = event.command.usr.id;

            const dpp::snowflake file_id = std::get<dpp::snowflake>(event.get_parameter("file"));
            const dpp::attachment att = event.command.get_resolved_attachment(file_id);

            auto r = co_await delta()->bot.co_request(att.url, dpp::http_method::m_get, "", att.content_type);

            dpp::message message = dpp::message("File uploaded:").add_file(att.filename, r.body, att.content_type).set_channel_id(event.command.channel_id).set_guild_id(event.command.guild_id);
            co_await waiting;
            co_await event.co_edit_response(message);
            auto co_mess = co_await event.co_get_original_response();

            if (co_mess.is_error()) {
                delta()->bot.log(dpp::loglevel::ll_debug, "Err update: " + co_mess.get_error().human_readable);
                event.co_edit_response("Failed to update element to database! Error: " + co_mess.get_error().human_readable);
                co_return;
            }
            dpp::message temp_msg = co_mess.get<dpp::message>();

            delta()->bot.log(dpp::loglevel::ll_debug, std::to_string(temp_msg.attachments.size()));
            delta()->bot.log(dpp::loglevel::ll_debug, temp_msg.attachments[0].url);
            std::string url = temp_msg.attachments[0].url;
            std::string name = std::get<std::string>(event.get_parameter("name"));

            //TODO I might need to "store" the original url to a dedicated channel to make the url work
            auto res1 = delta()->db.bind_parameter(saved_update_stmt, 0, saved_update_guild, guild_id);
            auto res2 = delta()->db.bind_parameter(saved_update_stmt, 0, saved_update_user, user_id);
            auto res3 = delta()->db.bind_parameter(saved_update_stmt, 0, saved_update_file_url, url.c_str(), url.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            auto res4 = delta()->db.bind_parameter(saved_update_stmt, 0, saved_update_file_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            mln::db_result res5;
            if (valid_desc) {
                res5 = delta()->db.bind_parameter(saved_update_stmt, 0, saved_update_desc, desc.c_str(), desc.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            }else {
                res5 = delta()->db.bind_parameter(saved_update_stmt, 0, saved_update_desc);
            }

            if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok || res4 != mln::db_result::ok || res5 != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind update params!" + name + " " + desc);
                temp_msg.set_content("Failed to update element, internal error!");
                event.edit_original_response(temp_msg);
                co_return;
            }

            auto res = delta()->db.exec(saved_update_stmt, mln::database_callbacks_t());
            if (res != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to update element!");
                temp_msg.set_content("Failed to update element, internal error!");
            }else {
                temp_msg.set_content("Element updated to the db!");
            }

            event.edit_original_response(temp_msg);
        }},
        {"update_description", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            auto waiting = event.co_thinking(true);//this one requires !broadcast since the condition wants true for ephemeral

            std::string desc;
            const dpp::command_value desc_param = event.get_parameter("description");
            const bool valid_desc = std::holds_alternative<std::string>(desc_param);
            if (valid_desc) {
                desc = std::get<std::string>(desc_param);
            }

            int64_t guild_id = event.command.guild_id;
            int64_t user_id = event.command.usr.id;

            dpp::message temp_msg;
            std::string name = std::get<std::string>(event.get_parameter("name"));

            //TODO I might need to "store" the original url to a dedicated channel to make the url work
            auto res1 = delta()->db.bind_parameter(saved_update_desc_stmt, 0, saved_update_desc_guild, guild_id);
            auto res2 = delta()->db.bind_parameter(saved_update_desc_stmt, 0, saved_update_desc_user, user_id);
            auto res3 = delta()->db.bind_parameter(saved_update_desc_stmt, 0, saved_update_desc_file_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            mln::db_result res4;
            if (valid_desc) {
                res4 = delta()->db.bind_parameter(saved_update_desc_stmt, 0, saved_update_desc_desc, desc.c_str(), desc.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            }else {
                res4 = delta()->db.bind_parameter(saved_update_desc_stmt, 0, saved_update_desc_desc);
            }

            if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok || res4 != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind update_desc params!" + name + " " + desc);
                temp_msg.set_content("Failed to update_desc element, internal error!");
                co_await waiting;
                event.edit_original_response(temp_msg);
                co_return;
            }

            auto res = delta()->db.exec(saved_update_desc_stmt, mln::database_callbacks_t());
            if (res != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to update_desc element!");
                temp_msg.set_content("Failed to update_desc element, internal error!");
            }else {
                temp_msg.set_content("Element updated to the db!");
            }

            co_await waiting;
            event.edit_original_response(temp_msg);
        }},
        {"remove", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            const dpp::command_value broadcast_param = event.get_parameter("broadcast");
            const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
            auto waiting = event.co_thinking(!broadcast);//this one requires !broadcast since the condition wants true for ephemeral

            int64_t guild_id = event.command.guild_id;
            int64_t user_id = event.command.usr.id;

            std::string name = std::get<std::string>(event.get_parameter("name"));

            auto res1 = delta()->db.bind_parameter(saved_remove_stmt, 0, saved_remove_guild, guild_id);
            auto res2 = delta()->db.bind_parameter(saved_remove_stmt, 0, saved_remove_user, user_id);
            auto res3 = delta()->db.bind_parameter(saved_remove_stmt, 0, saved_remove_file_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            dpp::message msg{};
            if (!broadcast) {
                msg.set_flags(dpp::m_ephemeral);
            }

            if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind remove params!");
                msg.set_content("Failed to remove element, internal error!");
                co_await waiting;
                event.edit_response(msg);
                co_return;
            }

            auto res = delta()->db.exec(saved_remove_stmt, mln::database_callbacks_t());
            if (res != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to remove element!");
                msg.set_content("Failed to remove element, internal error!");
            }
            else {
                msg.set_content("Element removed from the db!");
            }

            co_await waiting;
            event.edit_response(msg);
        },
    }};
    //TODO updates/modifications don't report an error when trying to modify records with the wrong user. I need to add functions in the db_callbacks to check if any elements are found
    //TODO insert_replace right now changes a present record even if the author is someone else, that should not be allowed
    static const std::unordered_map<std::string, op_callback_f> allowed_other_sub_commands{
        //TODO use another sub cmd group for all the commands that do not use the actual database (like set channel for attachment urls and such). op sub cmd group is for actual db stuff
    };
    static const std::unordered_map<std::string, const std::unordered_map<std::string, op_callback_f>*> allowed_primary_sub_commands{
        {"op", &allowed_op_sub_commands},
    };

    if (!valid_stmt) {
        delta()->bot.log(dpp::ll_error, "Failed db command, the query stmts have not been initialized correctly!");
        event.reply(dpp::message("Failed to execute the command, internal error!").set_flags(dpp::m_ephemeral));
        co_return;
    }

    /*if (delta()->is_dev_id_valid && event.command.usr.id != delta()->dev_id) {
        event.reply("These commands are temporarily disabled for normal users, wip!");
        co_return;
    }*/

    dpp::command_interaction cmd_data = event.command.get_command_interaction();
    dpp::command_data_option primary_cmd = cmd_data.options[0];
    const auto& it = allowed_primary_sub_commands.find(primary_cmd.name);
    if (it == allowed_primary_sub_commands.end()) {
        event.reply("Couldn't find primary sub_command " + primary_cmd.name);
        co_return;
    }

    const auto* const mapper = it->second;
    dpp::command_data_option sub_command = primary_cmd.options[0];
    const auto& sub_it = mapper->find(sub_command.name);
    if (sub_it == mapper->end()) {
        event.reply("Couldn't find " + primary_cmd.name + " sub_command " + sub_command.name);
        co_return;
    }

    co_await sub_it->second(sub_command, event);
}