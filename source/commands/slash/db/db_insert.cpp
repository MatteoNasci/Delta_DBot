#include "commands/slash/db/db_insert.h"
#include "database/database_handler.h"
#include "bot_delta.h"
#include "utility/utility.h"


mln::db_insert::db_insert(bot_delta* const delta, mln::db_insert::db_params_t&& in_data, const std::string& err) : base_db_command(delta), data(std::move(in_data)), err_msg(err) {}

mln::db_insert::db_insert(bot_delta* const delta) : base_db_command(delta), data({.valid_stmt = true}), err_msg("Failed while executing database query, record already present in the database!") {

    const mln::db_result res1 = delta->db.save_statement("INSERT OR ABORT INTO storage (guild_id, name, url, url_type, desc, user_id) VALUES(:GGG, :NNN, :LLL, :TTT, :DDD, :UUU) RETURNING user_id;", data.saved_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        data.valid_stmt = false;
    }else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":GGG", data.saved_param_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":NNN", data.saved_param_name);
        mln::db_result res13 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":LLL", data.saved_param_url);
        mln::db_result res14 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":TTT", data.saved_param_url_type);
        mln::db_result res15 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":UUU", data.saved_param_user);
        mln::db_result res16 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":DDD", data.saved_param_desc);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok || res14 != mln::db_result::ok || res15 != mln::db_result::ok || res16 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert stmt param indexes!");
            data.valid_stmt = false;
        }
    }
}

dpp::task<void> mln::db_insert::command(const dpp::command_data_option&, const dpp::slashcommand_t& event_data, mln::url_type type) {
    co_await mln::db_insert::exec_command(event_data, data, type, delta(), err_msg);
}

dpp::task<void> mln::db_insert::exec_command(const dpp::slashcommand_t& event_data, const mln::db_insert::db_params_t& params, mln::url_type type, mln::bot_delta* const delta, const std::string& db_execution_empty_err_msg) {
    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(true);

    if (!params.valid_stmt) {
        co_await thinking;
        event_data.edit_response("Failed database operation, the database was not initialized correctly!");
        co_return;
    }

    const dpp::snowflake guild_id = event_data.command.guild_id;
    uint64_t dump_channel;
    const bool valid_dump_channel = delta->get_dump_channel_id(guild_id, dump_channel);
    if (!valid_dump_channel) {
        dump_channel = event_data.command.channel_id;
    }

    const mln::db_insert::task_output_t data = co_await(type == mln::url_type::file ?
        mln::db_insert::file_command(event_data, mln::db_insert::task_input_t{.guild_id = guild_id, .dump_channel_id = dump_channel, .delta = delta}, thinking) :
        mln::db_insert::text_command(event_data, mln::db_insert::task_input_t{.guild_id = guild_id, .dump_channel_id = dump_channel, .delta = delta}, thinking));

    if (data.error) {
        co_return;
    }

    std::string desc;
    const dpp::command_value desc_param = event_data.get_parameter("description");
    const bool valid_description = std::holds_alternative<std::string>(desc_param);
    if (valid_description) {
        desc = std::get<std::string>(desc_param);
    }

    const dpp::snowflake user_id = event_data.command.usr.id;
    const std::string name = std::get<std::string>(event_data.get_parameter("name"));

    mln::db_result res1 = delta->db.bind_parameter(params.saved_stmt, 0, params.saved_param_guild, static_cast<int64_t>(guild_id));
    mln::db_result res2 = delta->db.bind_parameter(params.saved_stmt, 0, params.saved_param_user, static_cast<int64_t>(user_id));
    mln::db_result res3 = delta->db.bind_parameter(params.saved_stmt, 0, params.saved_param_url, data.url.c_str(), data.url.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
    mln::db_result res4 = delta->db.bind_parameter(params.saved_stmt, 0, params.saved_param_url_type, static_cast<int>(type));
    mln::db_result res5 = delta->db.bind_parameter(params.saved_stmt, 0, params.saved_param_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
    mln::db_result res6;
    if (valid_description) {
        res6 = delta->db.bind_parameter(params.saved_stmt, 0, params.saved_param_desc, desc.c_str(), desc.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);
    } else {
        res6 = delta->db.bind_parameter(params.saved_stmt, 0, params.saved_param_desc);
    }

    if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok || res4 != mln::db_result::ok || res5 != mln::db_result::ok || res6 != mln::db_result::ok) {
        delta->bot.message_delete(data.created_message_id, dump_channel);
        co_await thinking;
        event_data.edit_response("Failed to bind query parameters, internal error!");
        co_return;
    }

    bool db_success = false;
    mln::database_callbacks_t calls = mln::utility::get_any_results_callback();
    calls.callback_data = static_cast<void*>(&db_success);

    mln::db_result res = delta->db.exec(params.saved_stmt, calls);
    if (res != mln::db_result::ok || !db_success) {
        delta->bot.message_delete(data.created_message_id, dump_channel);
        co_await thinking;
        event_data.edit_response(res == mln::db_result::ok && !db_success ? db_execution_empty_err_msg : "Failed while executing database query, internal error!");
        co_return;
    }

    co_await thinking;
    event_data.edit_response("Database operation successful!");
}

dpp::task<mln::db_insert::task_output_t> mln::db_insert::file_command(const dpp::slashcommand_t& event_data, const mln::db_insert::task_input_t& input, dpp::async<dpp::confirmation_callback_t>& thinking) {
    const dpp::snowflake file_id = std::get<dpp::snowflake>(event_data.get_parameter("file"));
    const dpp::attachment att = event_data.command.get_resolved_attachment(file_id);

    const dpp::http_request_completion_t request_result = co_await input.delta->bot.co_request(att.url, dpp::http_method::m_get, "", att.content_type);

    mln::db_insert::task_output_t result{.error = true};
    if (request_result.status != 200) {
        co_await thinking;
        event_data.co_edit_response("Failed to download the attachment! Http error: " + std::to_string(request_result.error) + ", status: " + std::to_string(request_result.status));
        co_return result;
    }

    const dpp::message upload_message = dpp::message("File uploaded:").add_file(att.filename, request_result.body, att.content_type).set_channel_id(input.dump_channel_id).set_guild_id(input.guild_id);

    const dpp::confirmation_callback_t msg_create_result = co_await input.delta->bot.co_message_create(upload_message);

    if (msg_create_result.is_error()) {
        co_await thinking;
        event_data.edit_response("Failed to upload message with the attachment! Error: " + msg_create_result.get_error().human_readable);
        co_return result;
    }

    const dpp::message created_message = msg_create_result.get<dpp::message>();
    result.created_message_id = created_message.id;
    result.url = created_message.attachments.size() > 0 ? created_message.attachments[0].url : "";

    if (result.url.empty()) {
        input.delta->bot.message_delete(created_message.id, input.dump_channel_id);
        co_await thinking;
        event_data.edit_response("Failed to retrieve url!");
        co_return result;
    }

    result.error = false;
    co_return result;
}

dpp::task<mln::db_insert::task_output_t> mln::db_insert::text_command(const dpp::slashcommand_t& event_data, const mln::db_insert::task_input_t& input, dpp::async<dpp::confirmation_callback_t>& thinking) {
    const std::string text = std::get<std::string>(event_data.get_parameter("text_to_store")); //4096 limit

    dpp::embed embed{};
    embed.set_description(text);

    const dpp::message text_message = dpp::message(embed).set_channel_id(input.dump_channel_id).set_guild_id(input.guild_id);

    const dpp::confirmation_callback_t msg_create_result = co_await input.delta->bot.co_message_create(text_message);

    mln::db_insert::task_output_t result{.error = true};
    if (msg_create_result.is_error()) {
        co_await thinking;
        event_data.edit_response("Failed to upload message with the text content! Error: " + msg_create_result.get_error().human_readable);
        co_return result;
    }

    const dpp::message created_message = msg_create_result.get<dpp::message>();
    result.created_message_id = created_message.id;
    result.url = created_message.get_url();

    if (result.url.empty()) {
        input.delta->bot.message_delete(created_message.id, input.dump_channel_id);
        co_await thinking;
        event_data.edit_response("Failed to retrieve url!");
        co_return result;
    }

    result.error = false;
    co_return result;
}
