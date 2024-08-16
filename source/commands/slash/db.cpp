#include "commands/slash/db.h"
#include "bot_delta.h"
#include "utility/constants.h"
#include "utility/utility.h"

#include <dpp/queues.h>

mln::db::db(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("db", "Manage the database.", delta->bot.me.id)
        .add_option(dpp::command_option(dpp::co_sub_command_group, "op", "Perform an operation on the db", false)
            .add_option(dpp::command_option(dpp::co_sub_command, "insert", "Inserts a new record in the db. It will fail if the given name is not unique!", false)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the file.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_string, "description", "Small description of the stored file. Default: NULL", false)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))))
            .add_option(dpp::command_option(dpp::co_sub_command, "insert_update", "Inserts or replaces a record in the db.", false)
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
                .add_option(dpp::command_option(dpp::co_boolean, "verbose", "Tells the bot to output as much info as possible about the record. Default: false", false))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "show_records", "Shows all the db records names.", false)
                .add_option(dpp::command_option(dpp::co_boolean, "verbose", "Tells the bot to output as much info as possible about the records. Default: false", false))
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
            .add_option(dpp::command_option(dpp::co_sub_command, "delete", "Removes an existing record in the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the file.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))))) , 
    saved_insert_stmt(), saved_insert_replace_stmt(), saved_select_stmt(), saved_select_verbose_stmt(), saved_show_records_stmt(), saved_show_records_verbose_stmt(), saved_update_stmt(), saved_update_desc_stmt(), saved_remove_stmt(),
    saved_insert_guild(), saved_insert_user(), saved_insert_file_name(), saved_insert_file_url(), saved_insert_desc(),
    saved_insert_replace_guild(), saved_insert_replace_user(), saved_insert_replace_file_name(), saved_insert_replace_file_url(), saved_insert_replace_desc(),
    saved_select_guild(), saved_select_file_name(),
    saved_select_verbose_guild(), saved_select_verbose_file_name(),
    saved_update_guild(), saved_update_user(), saved_update_file_name(), saved_update_file_url(), saved_update_desc(),
    saved_update_desc_guild(), saved_update_desc_user(), saved_update_desc_file_name(), saved_update_desc_desc(),
    saved_remove_guild(), saved_remove_user(), saved_remove_file_name(), valid_stmt(true) {
    
    mln::db_result res1 = delta->db.save_statement("INSERT OR ABORT INTO file (guild_id, file_name, file_url, file_desc, user_id) VALUES(:GGG, :NNN, :FFF, :DDD, :UUU) RETURNING user_id;", saved_insert_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(saved_insert_stmt, 0, ":GGG", saved_insert_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(saved_insert_stmt, 0, ":NNN", saved_insert_file_name);
        mln::db_result res13 = delta->db.get_bind_parameter_index(saved_insert_stmt, 0, ":FFF", saved_insert_file_url);
        mln::db_result res14 = delta->db.get_bind_parameter_index(saved_insert_stmt, 0, ":UUU", saved_insert_user);
        mln::db_result res15 = delta->db.get_bind_parameter_index(saved_insert_stmt, 0, ":DDD", saved_insert_desc);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok || res14 != mln::db_result::ok || res15 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert stmt param indexes!");
            valid_stmt = false;
        }
    }

    res1 = delta->db.save_statement("INSERT INTO file (guild_id, file_name, file_url, file_desc, user_id) VALUES(:GGG, :NNN, :FFF, :DDD, :UUU) ON CONFLICT (guild_id, file_name) DO UPDATE SET file_url = excluded.file_url, file_desc = excluded.file_desc WHERE file.user_id = :UUU RETURNING user_id;", saved_insert_replace_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert_replace stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }
    else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(saved_insert_replace_stmt, 0, ":GGG", saved_insert_replace_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(saved_insert_replace_stmt, 0, ":NNN", saved_insert_replace_file_name);
        mln::db_result res13 = delta->db.get_bind_parameter_index(saved_insert_replace_stmt, 0, ":FFF", saved_insert_replace_file_url);
        mln::db_result res14 = delta->db.get_bind_parameter_index(saved_insert_replace_stmt, 0, ":UUU", saved_insert_replace_user);
        mln::db_result res15 = delta->db.get_bind_parameter_index(saved_insert_replace_stmt, 0, ":DDD", saved_insert_replace_desc);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok || res14 != mln::db_result::ok || res15 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert_replace stmt param indexes!");
            valid_stmt = false;
        }
    }

    res1 = delta->db.save_statement("SELECT * FROM file WHERE guild_id = :GGG AND file_name = :NNN;", saved_select_verbose_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save select verbose stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }
    else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(saved_select_verbose_stmt, 0, ":GGG", saved_select_verbose_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(saved_select_verbose_stmt, 0, ":NNN", saved_select_verbose_file_name);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save select verbose stmt param indexes!");
            valid_stmt = false;
        }
    }

    res1 = delta->db.save_statement("SELECT file_url, file_desc FROM file WHERE guild_id = :GGG AND file_name = :NNN;", saved_select_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save select stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }
    else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(saved_select_stmt, 0, ":GGG", saved_select_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(saved_select_stmt, 0, ":NNN", saved_select_file_name);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save select stmt param indexes!");
            valid_stmt = false;
        }
    }

    res1 = delta->db.save_statement("SELECT * FROM file WHERE guild_id = ?;", saved_show_records_verbose_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save select (show records) verbose stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }

    res1 = delta->db.save_statement("SELECT file_name, file_desc FROM file WHERE guild_id = ?;", saved_show_records_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save select (show records) stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }

    res1 = delta->db.save_statement("UPDATE OR ABORT file SET file_url = :FFF, file_desc = :DDD WHERE guild_id = :GGG AND file_name = :NNN AND user_id = :UUU RETURNING user_id;", saved_update_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save update stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }
    else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(saved_update_stmt, 0, ":GGG", saved_update_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(saved_update_stmt, 0, ":NNN", saved_update_file_name);
        mln::db_result res13 = delta->db.get_bind_parameter_index(saved_update_stmt, 0, ":FFF", saved_update_file_url);
        mln::db_result res14 = delta->db.get_bind_parameter_index(saved_update_stmt, 0, ":UUU", saved_update_user);
        mln::db_result res15 = delta->db.get_bind_parameter_index(saved_update_stmt, 0, ":DDD", saved_update_desc);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok || res14 != mln::db_result::ok || res15 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save update stmt param indexes!");
            valid_stmt = false;
        }
    }

    res1 = delta->db.save_statement("UPDATE OR ABORT file SET file_desc = :DDD WHERE guild_id = :GGG AND file_name = :NNN AND user_id = :UUU RETURNING user_id;", saved_update_desc_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save update_desc stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }
    else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(saved_update_desc_stmt, 0, ":GGG", saved_update_desc_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(saved_update_desc_stmt, 0, ":NNN", saved_update_desc_file_name);
        mln::db_result res13 = delta->db.get_bind_parameter_index(saved_update_desc_stmt, 0, ":UUU", saved_update_desc_user);
        mln::db_result res14 = delta->db.get_bind_parameter_index(saved_update_desc_stmt, 0, ":DDD", saved_update_desc_desc);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok || res14 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save update_desc stmt param indexes!");
            valid_stmt = false;
        }
    }

    res1 = delta->db.save_statement("DELETE FROM file WHERE guild_id = :GGG AND file_name = :NNN AND user_id = :UUU RETURNING user_id;", saved_remove_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save delete stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    }
    else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(saved_remove_stmt, 0, ":GGG", saved_remove_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(saved_remove_stmt, 0, ":NNN", saved_remove_file_name);
        mln::db_result res13 = delta->db.get_bind_parameter_index(saved_remove_stmt, 0, ":UUU", saved_remove_user);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save delete stmt param indexes!");
            valid_stmt = false;
        }
    }
}
dpp::task<void> mln::db::command(const dpp::slashcommand_t& event){//TODO put each sub command in its own .cpp file, too much shit here already (callbacks inside a .cpp interpreter as well)
    typedef std::function<dpp::task<void>(dpp::command_data_option&, const dpp::slashcommand_t&)> op_callback_f;
    static const std::unordered_map<std::string, op_callback_f> allowed_op_sub_commands{
        {"insert", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            dpp::async<dpp::confirmation_callback_t> thinking = event.co_thinking(false);

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

            const dpp::http_request_completion_t r = co_await delta()->bot.co_request(att.url, dpp::http_method::m_get, "", att.content_type);

            if (r.status != 200) {
                co_await thinking;
                event.co_edit_response("Failed to download the attachment!");
                co_return;
            }

            dpp::message message = dpp::message("File uploaded:").add_file(att.filename, r.body, att.content_type).set_channel_id(event.command.channel_id).set_guild_id(event.command.guild_id);
            co_await thinking;
            co_await event.co_edit_response(message);
            const dpp::confirmation_callback_t co_mess = co_await event.co_get_original_response();

            if (co_mess.is_error()) {
                delta()->bot.log(dpp::loglevel::ll_debug, "Err insert: " + co_mess.get_error().human_readable);
                event.co_edit_response("Failed to insert element to database! Error: " + co_mess.get_error().human_readable);
                co_return;
            }
            dpp::message temp_msg = co_mess.get<dpp::message>();
            
            std::string url = temp_msg.attachments.size() > 0 ? temp_msg.attachments[0].url : "";
            std::string name = std::get<std::string>(event.get_parameter("name"));

            if (url.empty()) {
                delta()->bot.log(dpp::loglevel::ll_debug, "Failed to retrieve url!");
                event.co_edit_response("Failed to retrieve url!");
                co_return;
            }
            
            //TODO Add an optional dump channel to send all the uploaded files, only the server owner can set/modify the targetted channel. This will prevent cluttering from all the uploaded files in the channel the commands are used from.
            mln::db_result res1 = delta()->db.bind_parameter(saved_insert_stmt, 0, saved_insert_guild, guild_id);
            mln::db_result res2 = delta()->db.bind_parameter(saved_insert_stmt, 0, saved_insert_user, user_id);
            mln::db_result res3 = delta()->db.bind_parameter(saved_insert_stmt, 0, saved_insert_file_url, url.c_str(), url.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            mln::db_result res4 = delta()->db.bind_parameter(saved_insert_stmt, 0, saved_insert_file_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
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
                event.edit_response("Failed to insert element, internal error!");
                co_return;
            }

            bool db_success = false;
            mln::database_callbacks_t calls{};
            calls.callback_data = &db_success;
            calls.data_adder_callback = [](void* d, int, mln::db_column_data_t&&) {
                bool* b_ptr = static_cast<bool*>(d);
                *b_ptr = true;
            };
            calls.type_definer_callback = [](void*, int) { return false; };

            mln::db_result res = delta()->db.exec(saved_insert_stmt, calls);
            if (res != mln::db_result::ok || !db_success) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to insert element!");
                temp_msg = dpp::message(res == mln::db_result::ok && !db_success ? "Failed to insert element, record already present in the database!" : "Failed to insert element, internal error!");
            }else {
                temp_msg.set_content("Element inserted to the db!");
            }

            event.edit_response(temp_msg);
        }},
        {"insert_update", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            dpp::async<dpp::confirmation_callback_t> thinking = event.co_thinking(false);//this one requires !broadcast since the condition wants true for ephemeral

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

            const dpp::http_request_completion_t r = co_await delta()->bot.co_request(att.url, dpp::http_method::m_get, "", att.content_type);

            if (r.status != 200) {
                co_await thinking;
                event.co_edit_response("Failed to download the attachment!");
                co_return;
            }

            dpp::message message = dpp::message("File uploaded:").add_file(att.filename, r.body, att.content_type).set_channel_id(event.command.channel_id).set_guild_id(event.command.guild_id);
            co_await thinking;
            co_await event.co_edit_response(message);
            const dpp::confirmation_callback_t co_mess = co_await event.co_get_original_response();

            if (co_mess.is_error()) {
                delta()->bot.log(dpp::loglevel::ll_debug, "Err insert or update: " + co_mess.get_error().human_readable);
                event.co_edit_response("Failed to insert or update element to database! Error: " + co_mess.get_error().human_readable);
                co_return;
            }
            dpp::message temp_msg = co_mess.get<dpp::message>();

            std::string url = temp_msg.attachments.size() > 0 ? temp_msg.attachments[0].url : "";
            std::string name = std::get<std::string>(event.get_parameter("name"));

            if (url.empty()) {
                delta()->bot.log(dpp::loglevel::ll_debug, "Failed to retrieve url!");
                event.co_edit_response("Failed to retrieve url!");
                co_return;
            }

            mln::db_result res1 = delta()->db.bind_parameter(saved_insert_replace_stmt, 0, saved_insert_replace_guild, guild_id);
            mln::db_result res2 = delta()->db.bind_parameter(saved_insert_replace_stmt, 0, saved_insert_replace_user, user_id);
            mln::db_result res3 = delta()->db.bind_parameter(saved_insert_replace_stmt, 0, saved_insert_replace_file_url, url.c_str(), url.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            mln::db_result res4 = delta()->db.bind_parameter(saved_insert_replace_stmt, 0, saved_insert_replace_file_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            mln::db_result res5;
            if (valid_desc) {
                res5 = delta()->db.bind_parameter(saved_insert_replace_stmt, 0, saved_insert_replace_desc, desc.c_str(), desc.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            }
            else {
                res5 = delta()->db.bind_parameter(saved_insert_replace_stmt, 0, saved_insert_replace_desc);
            }

            if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok || res4 != mln::db_result::ok || res5 != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind insert_update params!" + name + " " + desc);
                event.edit_response("Failed to insert_update element, internal error!");
                co_return;
            }

            bool db_success = false;
            mln::database_callbacks_t calls{};
            calls.callback_data = &db_success;
            calls.data_adder_callback = [](void* d, int, mln::db_column_data_t&&) {
                bool* b_ptr = static_cast<bool*>(d);
                *b_ptr = true;
            };
            calls.type_definer_callback = [](void*, int) { return false; };

            mln::db_result res = delta()->db.exec(saved_insert_replace_stmt, calls);
            if (res != mln::db_result::ok || !db_success) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to insert_update element!");
                temp_msg = dpp::message(res == mln::db_result::ok && !db_success ? "Failed to insert_update element, the record is already present but you are not allowed to modify it!" : "Failed to insert_update element, internal error!");
            }else {
                temp_msg.set_content("Element inserted_updated to the db!");
            }

            event.edit_response(temp_msg);
        }},
        {"select", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            const dpp::command_value broadcast_param = event.get_parameter("broadcast");
            const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
            dpp::async<dpp::confirmation_callback_t> thinking = event.co_thinking(!broadcast);//this one requires !broadcast since the condition wants true for ephemeral

            const dpp::command_value verbose_param = event.get_parameter("verbose");
            const bool verbose = std::holds_alternative<bool>(verbose_param) ? std::get<bool>(verbose_param) : false;

            std::string name = std::get<std::string>(event.get_parameter("name"));

            mln::db_result res1, res2;
            if (verbose) {
                res1 = delta()->db.bind_parameter(saved_select_verbose_stmt, 0, saved_select_verbose_guild, static_cast<int64_t>(event.command.guild_id));
                res2 = delta()->db.bind_parameter(saved_select_verbose_stmt, 0, saved_select_verbose_file_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            }
            else {
                res1 = delta()->db.bind_parameter(saved_select_stmt, 0, saved_select_guild, static_cast<int64_t>(event.command.guild_id));
                res2 = delta()->db.bind_parameter(saved_select_stmt, 0, saved_select_file_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            }

            dpp::message msg{};
            if (!broadcast) {
                msg.set_flags(dpp::m_ephemeral);
            }

            if (res1 != mln::db_result::ok || res2 != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind select params!");
                msg.set_content("Failed to select, internal error!");
                co_await thinking;
                event.edit_response(msg);
                co_return;
            }

            mln::database_callbacks_t callbacks{};
            std::string ret_desc{};
            std::string ret_url{};
            std::string ret_u_id{};
            std::string ret_c_time{};
            bool found = false;
            callbacks.data_adder_callback = [verbose, &found, &ret_url, &ret_desc, &ret_u_id, &ret_c_time](void*, int c, mln::db_column_data_t&& d) {
                found = true;
                if (!verbose) {
                    if (c == 0) {
                        ret_url = std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data)));
                    }
                    else if (std::holds_alternative<const unsigned char*>(d.data)) {
                        ret_desc = std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data)));
                    }
                    return;
                }

                switch (c) {
                case 0:
                    break;
                case 1:
                    break;
                case 2:
                    ret_url = std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data)));
                    break;
                case 3:
                    ret_desc = std::holds_alternative<const unsigned char*>(d.data) ? std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data))) : "NULL";
                    break;
                case 4:
                    ret_u_id = std::to_string(static_cast<uint64_t>(std::get<int64_t>(d.data)));
                    break;
                case 5:
                    ret_c_time = std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data)));
                    break;
                default:
                    break;
                }
            };
            callbacks.type_definer_callback = [verbose](void*, int c) {return (verbose ? c != 0 && c != 4 : true); };        

            mln::db_result res = delta()->db.exec(verbose ? saved_select_verbose_stmt : saved_select_stmt, callbacks);
            if (res != mln::db_result::ok || !found) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to select!");
                msg.set_content(res == mln::db_result::ok && !found ? "Failed to select, no record found!" : "Failed to select, internal error!");
            }
            else {
                msg.set_content(verbose ? ("name: " + name + ", url: " + ret_url + ", desc: " + (ret_desc.empty() ? "NULL" : ret_desc) + ", user: " + ret_u_id + ", creation: " + ret_c_time) : ret_url + (ret_desc.empty() ? "" : ", " + ret_desc));
            }

            co_await thinking;
            event.edit_response(msg);
        }},
        {"show_records", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            const dpp::command_value broadcast_param = event.get_parameter("broadcast");
            const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
            dpp::async<dpp::confirmation_callback_t> thinking = event.co_thinking(!broadcast);//this one requires !broadcast since the condition wants true for ephemeral

            const dpp::command_value verbose_param = event.get_parameter("verbose");
            const bool verbose = std::holds_alternative<bool>(verbose_param) ? std::get<bool>(verbose_param) : false;

            mln::db_result res = verbose ? delta()->db.bind_parameter(saved_show_records_verbose_stmt, 0, 1, static_cast<int64_t>(event.command.guild_id)) : delta()->db.bind_parameter(saved_show_records_stmt, 0, 1, static_cast<int64_t>(event.command.guild_id));
            dpp::message msg{};
            if (!broadcast) {
                msg.set_flags(dpp::m_ephemeral);
            }

            if (res != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind show_records params!");
                msg.set_content("Failed to show_records, internal error!");
                co_await thinking;
                event.edit_response(msg);
                co_return;
            }

            mln::database_callbacks_t callbacks{};
            size_t current_index = 0;
            std::vector<std::tuple<std::string, std::string>> n_v_list;
            std::vector<std::tuple<std::string, std::string, std::string, std::string>> v_list;
            callbacks.data_adder_callback = [verbose, &n_v_list, &v_list, &current_index](void*, int c, mln::db_column_data_t&& d) {
                if (!verbose) {
                    if (c == 0) {
                        current_index = n_v_list.size();
                        n_v_list.push_back(std::make_tuple(std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data))), std::string()));
                    }
                    else {
                        if (std::holds_alternative<const unsigned char*>(d.data)) {
                            std::get<1>(n_v_list[current_index]) = std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data)));
                        }
                    }
                    return;
                }

                switch (c) {
                case 0:
                    break;
                case 1:
                    current_index = v_list.size();
                    v_list.push_back(std::make_tuple(std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data))), std::string(), std::string(), std::string()));
                    break;
                case 2:
                    break;
                case 3:
                    std::get<1>(v_list[current_index]) = std::holds_alternative<const unsigned char*>(d.data) ? std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data))) : "NULL";
                    break;
                case 4:
                    std::get<2>(v_list[current_index]) = std::to_string(static_cast<uint64_t>(std::get<int64_t>(d.data)));
                    break;
                case 5:
                    std::get<3>(v_list[current_index]) = std::string(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data)));
                    break;
                default:
                    break;
                }
            };
            callbacks.type_definer_callback = [verbose](void*, int c) {return (verbose ? c != 0 && c != 4 : true); };

            res = delta()->db.exec(verbose ? saved_show_records_verbose_stmt : saved_show_records_stmt, callbacks);
            if (res != mln::db_result::ok || (n_v_list.empty() && v_list.empty())) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to show_records!");
                msg.set_content(res == mln::db_result::ok && (n_v_list.empty() && v_list.empty()) ? "Failed to show_records, empty database!" : "Failed to show_records, internal error!");
                co_await thinking;
                event.edit_response(msg);
            }
            else {
                std::string s{};
                if (verbose) {
                    for (size_t i = 0; i < v_list.size(); ++i) {
                        const std::tuple<std::string, std::string, std::string, std::string>& tup = v_list[i];
                        s += "{ name: " + std::get<0>(tup) + ", desc: " + std::get<1>(tup) + ", usr: " + std::get<2>(tup) + ", time: " + std::get<3>(tup) + " }\n";
                    }
                }
                else {
                    for (size_t i = 0; i < n_v_list.size(); ++i) {
                        const std::tuple<std::string, std::string>& tup = n_v_list[i];
                        s += "{ " + std::get<0>(tup) + (std::get<1>(tup).empty() ? " }\n" : ", desc: " + std::get<1>(tup) + " }\n");
                    }
                }

                size_t current_index = 0;
                const std::function<std::string(size_t, size_t)> v_func = [&current_index, &v_list](size_t, size_t) {
                    if (current_index >= v_list.size()) {
                        return std::string{};
                    }
                    const std::tuple<std::string, std::string, std::string, std::string>& tup = v_list[current_index++];
                    return "{ name: " + std::get<0>(tup) + ", desc: " + std::get<1>(tup) + ", usr: " + std::get<2>(tup) + ", time: " + std::get<3>(tup) + " }\n";
                };
                const std::function<std::string(size_t, size_t)> n_v_func = [&current_index, &n_v_list](size_t, size_t) {
                    if (current_index >= n_v_list.size()) {
                        return std::string{};
                    }
                    const std::tuple<std::string, std::string>& tup = n_v_list[current_index++];
                    return "{ " + std::get<0>(tup) + (std::get<1>(tup).empty() ? " }\n" : ", desc: " + std::get<1>(tup) + " }\n");
                };

                co_await thinking;
                bool result = co_await mln::utility::send_msg_recursively_embed(delta()->bot, event, verbose ? v_func : n_v_func, event.command.usr.id, true, broadcast);
                if (!result) {
                    delta()->bot.log(dpp::loglevel::ll_error, "An error occurred while sending embeds recursively for /db op show_records command!");
                }
            }
        }},
        {"update", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            dpp::async<dpp::confirmation_callback_t> thinking = event.co_thinking(false);//this one requires !broadcast since the condition wants true for ephemeral

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

            const dpp::http_request_completion_t r = co_await delta()->bot.co_request(att.url, dpp::http_method::m_get, "", att.content_type);

            if (r.status != 200) {
                co_await thinking;
                event.co_edit_response("Failed to download the attachment!");
                co_return;
            }

            dpp::message message = dpp::message("File uploaded:").add_file(att.filename, r.body, att.content_type).set_channel_id(event.command.channel_id).set_guild_id(event.command.guild_id);
            co_await thinking;
            co_await event.co_edit_response(message);
            const dpp::confirmation_callback_t co_mess = co_await event.co_get_original_response();

            if (co_mess.is_error()) {
                delta()->bot.log(dpp::loglevel::ll_debug, "Err update: " + co_mess.get_error().human_readable);
                event.co_edit_response("Failed to update element to database! Error: " + co_mess.get_error().human_readable);
                co_return;
            }
            dpp::message temp_msg = co_mess.get<dpp::message>();

            std::string url = temp_msg.attachments.size() > 0 ? temp_msg.attachments[0].url : "";
            std::string name = std::get<std::string>(event.get_parameter("name"));

            if (url.empty()) {
                delta()->bot.log(dpp::loglevel::ll_debug, "Failed to retrieve url!");
                event.co_edit_response("Failed to retrieve url!");
                co_return;
            }

            mln::db_result res1 = delta()->db.bind_parameter(saved_update_stmt, 0, saved_update_guild, guild_id);
            mln::db_result res2 = delta()->db.bind_parameter(saved_update_stmt, 0, saved_update_user, user_id);
            mln::db_result res3 = delta()->db.bind_parameter(saved_update_stmt, 0, saved_update_file_url, url.c_str(), url.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            mln::db_result res4 = delta()->db.bind_parameter(saved_update_stmt, 0, saved_update_file_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            mln::db_result res5;
            if (valid_desc) {
                res5 = delta()->db.bind_parameter(saved_update_stmt, 0, saved_update_desc, desc.c_str(), desc.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            }else {
                res5 = delta()->db.bind_parameter(saved_update_stmt, 0, saved_update_desc);
            }

            if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok || res4 != mln::db_result::ok || res5 != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind update params!" + name + " " + desc);
                event.edit_response("Failed to update element, internal error!");
                co_return;
            }

            bool db_success = false;
            mln::database_callbacks_t calls{};
            calls.callback_data = &db_success;
            calls.data_adder_callback = [](void* d, int, mln::db_column_data_t&&) {
                bool* b_ptr = static_cast<bool*>(d);
                *b_ptr = true;
            };
            calls.type_definer_callback = [](void*, int) { return false; };

            mln::db_result res = delta()->db.exec(saved_update_stmt, calls);
            if (res != mln::db_result::ok || !db_success) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to update element!");
                temp_msg = dpp::message(res == mln::db_result::ok && !db_success ? "Failed to update element, either no record found in the database with the given name or you are not allowed to modify it!" : "Failed to update element, internal error!");
            }else {
                temp_msg.set_content("Element updated to the db!");
            }

            event.edit_response(temp_msg);
        }},
        {"update_description", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            dpp::async<dpp::confirmation_callback_t> thinking = event.co_thinking(true);//this one requires !broadcast since the condition wants true for ephemeral

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

            mln::db_result res1 = delta()->db.bind_parameter(saved_update_desc_stmt, 0, saved_update_desc_guild, guild_id);
            mln::db_result res2 = delta()->db.bind_parameter(saved_update_desc_stmt, 0, saved_update_desc_user, user_id);
            mln::db_result res3 = delta()->db.bind_parameter(saved_update_desc_stmt, 0, saved_update_desc_file_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            mln::db_result res4;
            if (valid_desc) {
                res4 = delta()->db.bind_parameter(saved_update_desc_stmt, 0, saved_update_desc_desc, desc.c_str(), desc.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            }else {
                res4 = delta()->db.bind_parameter(saved_update_desc_stmt, 0, saved_update_desc_desc);
            }

            if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok || res4 != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind update_desc params!" + name + " " + desc);
                co_await thinking;
                event.edit_response("Failed to update_desc element, internal error!");
                co_return;
            }

            bool db_success = false;
            mln::database_callbacks_t calls{};
            calls.callback_data = &db_success;
            calls.data_adder_callback = [](void* d, int, mln::db_column_data_t&&) {
                bool* b_ptr = static_cast<bool*>(d);
                *b_ptr = true;
            };
            calls.type_definer_callback = [](void*, int) { return false; };

            mln::db_result res = delta()->db.exec(saved_update_desc_stmt, calls);
            if (res != mln::db_result::ok || !db_success) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to update_desc element!");
                temp_msg.set_content(res == mln::db_result::ok && !db_success ? "Failed to update_desc element, either no record found in the database with the given name or you are not allowed to modify it!" : "Failed to update_desc element, internal error!");
            }else {
                temp_msg.set_content("Element updated to the db!");
            }

            co_await thinking;
            event.edit_response(temp_msg);
        }},
        {"delete", [this](dpp::command_data_option& opt, const dpp::slashcommand_t& event) -> dpp::task<void> {
            const dpp::command_value broadcast_param = event.get_parameter("broadcast");
            const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
            dpp::async<dpp::confirmation_callback_t> thinking = event.co_thinking(!broadcast);//this one requires !broadcast since the condition wants true for ephemeral

            int64_t guild_id = event.command.guild_id;
            int64_t user_id = event.command.usr.id;

            std::string name = std::get<std::string>(event.get_parameter("name"));

            mln::db_result res1 = delta()->db.bind_parameter(saved_remove_stmt, 0, saved_remove_guild, guild_id);
            mln::db_result res2 = delta()->db.bind_parameter(saved_remove_stmt, 0, saved_remove_user, user_id);
            mln::db_result res3 = delta()->db.bind_parameter(saved_remove_stmt, 0, saved_remove_file_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
            
            if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind delete params!");
                co_await thinking;
                event.edit_response("Failed to delete element, internal error!");
                co_return;
            }

            bool db_success = false;
            mln::database_callbacks_t calls{};
            calls.callback_data = &db_success;
            calls.data_adder_callback = [](void* d, int, mln::db_column_data_t&&) {
                bool* b_ptr = static_cast<bool*>(d);
                *b_ptr = true;
            };
            calls.type_definer_callback = [](void*, int) { return false; };

            dpp::message msg{};
            mln::db_result res = delta()->db.exec(saved_remove_stmt, calls);
            if (res != mln::db_result::ok || !db_success) {
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to delete element!");
                msg.set_content(res == mln::db_result::ok && !db_success ? "Failed to delete element, either no record found with the given name or you are not allowed to modify the record!" : "Failed to delete element, internal error!");
            }
            else {
                msg.set_content("Element deleted from the db!");
            }

            co_await thinking;
            event.edit_response(msg);
        },
    }};

    static const std::unordered_map<std::string, op_callback_f> allowed_other_sub_commands{
        //TODO use another sub cmd group for all the commands that do not use the actual database (like set channel for attachment urls and such). op sub cmd group is for actual db stuff
    };
    static const std::unordered_map<std::string, const std::unordered_map<std::string, op_callback_f>*> allowed_primary_sub_commands{
        {"op", &allowed_op_sub_commands},
    };
    
    //TODO maybe make it so that the sub cmd return dpp::message? (for when i'll separate the commands into their own files)
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
    
    const std::unordered_map<std::string, op_callback_f>* const mapper = it->second;
    dpp::command_data_option sub_command = primary_cmd.options[0];
    const auto& sub_it = mapper->find(sub_command.name);
    if (sub_it == mapper->end()) {
        event.reply("Couldn't find " + primary_cmd.name + " sub_command " + sub_command.name);
        co_return;
    }

    co_await sub_it->second(sub_command, event);
}