#include "bot_delta.h"
#include "defines.h"
#include "database/database_handler.h"
#include "database/database_callbacks.h"
#include "database/db_column_data.h"

#include <dpp/dpp.h>
#include <dpp/intents.h>

static const std::string s_create_main_table("CREATE TABLE IF NOT EXISTS main.guild_profile( guild_id INTEGER PRIMARY KEY, db_channel INTEGER);");
static const std::string s_insert_new_guild("INSERT OR IGNORE INTO main.guild_profile (guild_id, db_channel) VALUES(? , NULL);");
static const std::string s_update_guild_db_channel("UPDATE main.guild_profile SET db_channel=:CCC WHERE guild_id=:GGG;");
static const std::string s_select_guild_db_channel("SELECT DISTINCT db_channel FROM main.guild_profile WHERE guild_id=?;");
static const std::string s_select_all("SELECT * FROM main.guild_profile;");

mln::bot_delta_data_t::bot_delta_data_t(bot_delta* in_delta, dpp::snowflake in_dev_id, bool in_is_dev_id_valid, bool in_register_commands) :
    delta(in_delta),
    bot(DISCORD_BOT_TOKEN, dpp::i_default_intents | dpp::i_guild_members | dpp::i_message_content), 
    dev_id(in_dev_id), 
    is_dev_id_valid(in_is_dev_id_valid),
    registered_new_cmds(in_register_commands){}

void mln::bot_delta::setup_db() {
    mln::db_result res = db.open_connection("data.db");
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        err_msg = "An error occurred while connecting to database: " + err_msg;
        throw std::exception(err_msg.c_str());
    }

    res = db.exec(s_create_main_table, mln::database_callbacks_t());
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        err_msg = "An error occurred while creating the guild_profile table: " + err_msg;
        throw std::exception(err_msg.c_str());
    }

    res = db.save_statement(s_insert_new_guild, saved_insert_guild_query);
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        err_msg = "An error occurred while saving the insert guild statement: " + err_msg;
        throw std::exception(err_msg.c_str());
    }

    res = db.save_statement(s_update_guild_db_channel, saved_update_guild_channel_query);
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        err_msg = "An error occurred while saving the update guild channel statement: " + err_msg;
        throw std::exception(err_msg.c_str());
    }

    res = db.get_bind_parameter_index(saved_update_guild_channel_query, 0, ":GGG", saved_ugc_guild_index);
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        err_msg = "An error occurred while saving the guild index for update guild channel statement: " + err_msg;
        throw std::exception(err_msg.c_str());
    }

    res = db.get_bind_parameter_index(saved_update_guild_channel_query, 0, ":CCC", saved_ugc_channel_index);
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        err_msg = "An error occurred while saving the channel index for update guild channel statement: " + err_msg;
        throw std::exception(err_msg.c_str());
    }

    res = db.save_statement(s_update_guild_db_channel, saved_update_guild_channel_query);
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        err_msg = "An error occurred while saving the update guild channel statement: " + err_msg;
        throw std::exception(err_msg.c_str());
    }

    res = db.save_statement(s_select_guild_db_channel, saved_select_guild_channel_query);
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        err_msg = "An error occurred while saving the select guild channel statement: " + err_msg;
        throw std::exception(err_msg.c_str());
    }

    res = db.save_statement(s_select_all, saved_select_all_query);
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        err_msg = "An error occurred while saving the select all statement: " + err_msg;
        throw std::exception(err_msg.c_str());
    }

    data.bot.log(dpp::loglevel::ll_debug, "Main db successfully created database table and saved queries!");

    mln::bot_delta::print_main_db();
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        err_msg = "An error occurred while printing main db: " + err_msg;
        throw std::exception(err_msg.c_str());
    }
}

void mln::bot_delta::init(){
    data.bot.on_log(dpp::utility::cout_logger());
    data.bot.log(dpp::loglevel::ll_info, data.is_dev_id_valid ? "Dev id found!" : "Dev id not found!");

    mln::bot_delta::setup_db();

    readys.attach_event(data);
    cmds.attach_event(data);
    ctxs.attach_event(data);
    forms.attach_event(data);
    selects.attach_event(data);
    react_removes.attach_event(data);
    msg_creates.attach_event(data);
    button_clicks.attach_event(data);
    autocompletes.attach_event(data);
}

void mln::bot_delta::initialize_environment(){
    const mln::db_result res = mln::database_handler::initialize_db_environment();
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        err_msg = "An error occurred while initializing database environment: " + err_msg;
        throw std::exception(err_msg.c_str());
    }
}

void mln::bot_delta::shutdown_environment(){
    const mln::db_result res = mln::database_handler::shutdown_db_environment();
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        err_msg = "An error occurred during database environment shutdown: " + err_msg;
        throw std::exception(err_msg.c_str());
    }
}


mln::bot_delta::~bot_delta(){
    
}

mln::bot_delta::bot_delta(const bool register_cmds) :
    data( 
    this,
    dpp::snowflake(DISCORD_DEV_ID), 
#ifdef MLN_DB_DISCORD_DEV_ID
    true,
#else //MLN_DB_DISCORD_DEV_ID
    false,
#endif //MLN_DB_DISCORD_DEV_ID
    register_cmds),
    db(),
    saved_insert_guild_query(),
    saved_update_guild_channel_query(),
    saved_ugc_guild_index(),
    saved_ugc_channel_index(),
    saved_select_guild_channel_query(),
    saved_select_all_query(),
    cmds(), 
    ctxs(), 
    forms(), 
    selects(),
    readys(),
    react_removes(),
    msg_creates(),
    button_clicks(),
    autocompletes()
{
    this->init();
}

std::string mln::bot_delta::start(){
    data.bot.start(dpp::st_return);
    return "Bot deployed!";
}

bool mln::bot_delta::close(){
    db.delete_statement(saved_insert_guild_query);

    db.delete_statement(saved_update_guild_channel_query);

    db.delete_statement(saved_select_guild_channel_query);

    return db.close_connection() == mln::db_result::ok;
}

mln::db_result mln::bot_delta::insert_new_guild_id(const dpp::snowflake& guild_id){
    db.bind_parameter(saved_insert_guild_query, 0, 1, static_cast<int64_t>(guild_id));
    return db.exec(saved_insert_guild_query, mln::database_callbacks_t());
}

mln::db_result mln::bot_delta::update_guild_db_channel_id(const dpp::snowflake& guild_id, const dpp::snowflake& channel_id){
    db.bind_parameter(saved_update_guild_channel_query, 0, saved_ugc_channel_index, static_cast<int64_t>(channel_id));
    db.bind_parameter(saved_update_guild_channel_query, 0, saved_ugc_guild_index, static_cast<int64_t>(guild_id));
    return db.exec(saved_update_guild_channel_query, mln::database_callbacks_t());
}

mln::db_result mln::bot_delta::select_guild_db_channel_id(const dpp::snowflake& guild_id, bool& valid_channel, dpp::snowflake& out_channel_id) {
    std::pair<bool, dpp::snowflake> pair(false, 0);
    valid_channel = false;
    db.bind_parameter(saved_select_guild_channel_query, 0, 1, static_cast<int64_t>(guild_id));
    const mln::db_result res = db.exec(saved_select_guild_channel_query, mln::bot_delta::get_select_callbacks(pair));

    valid_channel = pair.first;
    out_channel_id = pair.second;

    return res;
}

bool row_call(void* data) {
    return true;
}
void da_call(void* data, int, mln::db_column_data_t&& column_data) {
    std::pair<bool, dpp::snowflake>* pair = (static_cast<std::pair<bool, dpp::snowflake>*>(data));
    pair->first = !std::holds_alternative<const short*>(column_data.data);
    if (pair->first) {
        pair->second = static_cast<dpp::snowflake>(std::get<int64_t>(column_data.data));
    }
}
void da_all_call(void* data, int col, mln::db_column_data_t&& column_data) {
    mln::bot_delta* delta = static_cast<mln::bot_delta*>(data);
    if (col == 0) {
        delta->data.bot.log(dpp::loglevel::ll_debug, "guild_id = " + std::to_string(static_cast<uint64_t>(std::get<int64_t>(column_data.data))));
    }else {
        const bool holds_null = std::holds_alternative<const short*>(column_data.data);
        delta->data.bot.log(dpp::loglevel::ll_debug, "channel_id = " + (!holds_null ? std::to_string(static_cast<uint64_t>(std::get<int64_t>(column_data.data))) : "NULL") + "\n");
    }
}
bool td_call(int) {
    return false;
}
void stmt_i_call(void*, size_t) {
    
}

mln::database_callbacks_t mln::bot_delta::get_select_callbacks(std::pair<bool, dpp::snowflake>& res) {
    mln::database_callbacks_t calls;
    calls.row_callback = &row_call;
    calls.data_adder_callback = &da_call;
    calls.type_definer_callback = &td_call;
    calls.statement_index_callback = nullptr;
    calls.callback_data = static_cast<void*>(&res);
    return calls;
}

mln::database_callbacks_t mln::bot_delta::get_select_all_callbacks() {
    mln::database_callbacks_t calls;
    calls.row_callback = &row_call;
    calls.data_adder_callback = &da_all_call;
    calls.type_definer_callback = &td_call;
    calls.statement_index_callback = nullptr;
    calls.callback_data = this;
    return calls;
}

mln::db_result mln::bot_delta::print_main_db(){
    data.bot.log(dpp::ll_debug, "Printing main db...");
    const mln::db_result res = db.exec(saved_select_all_query, mln::bot_delta::get_select_all_callbacks());
    data.bot.log(dpp::ll_debug, "Printing over.");

    return res;
}