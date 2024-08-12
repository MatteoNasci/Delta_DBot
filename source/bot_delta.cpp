#include "bot_delta.h"
#include "defines.h"
#include "utility/constants.h"

static const std::string s_execute_pragmas("PRAGMA foreign_keys = ON;");
static const std::string s_create_main_table("CREATE TABLE IF NOT EXISTS guild_profile( guild_id INTEGER PRIMARY KEY NOT NULL);");
//primary key should be both the foreign key and file_name, to allow different guilds to use same name
static const std::string s_create_file_table("CREATE TABLE IF NOT EXISTS file( guild_id INTEGER NOT NULL REFERENCES guild_profile(guild_id), file_name TEXT NOT NULL CHECK (LENGTH(file_name) >= " 
    + std::to_string(static_cast<int64_t>(mln::constants::get_min_characters_text_id())) + " AND LENGTH(file_name) <= " 
    + std::to_string(static_cast<int64_t>(mln::constants::get_max_characters_text_id())) + "), file_id INTEGER NOT NULL, PRIMARY KEY(guild_id, file_name));");

static const std::string s_select_all("SELECT * FROM guild_profile; SELECT * FROM file;");
void mln::bot_delta::setup_db() {
    mln::db_result res = db.open_connection("dbs/main.db");
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while connecting to database: " + mln::database_handler::get_name_from_result(res);
        throw std::exception(err_msg.c_str());
    }

    res = db.exec(s_execute_pragmas, mln::database_callbacks_t());
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while executing pragmas: " + mln::database_handler::get_name_from_result(res);
        throw std::exception(err_msg.c_str());
    }

    res = db.exec(s_create_main_table, mln::database_callbacks_t());
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while creating the guild_profile table: " + mln::database_handler::get_name_from_result(res);
        throw std::exception(err_msg.c_str());
    }

    res = db.exec(s_create_file_table, mln::database_callbacks_t());
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while creating the file table: " + mln::database_handler::get_name_from_result(res);
        throw std::exception(err_msg.c_str());
    }

    res = db.save_statement(s_select_all, saved_select_all_query);
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while saving the select all stmt: " + mln::database_handler::get_name_from_result(res);
        throw std::exception(err_msg.c_str());
    }

    bot.log(dpp::loglevel::ll_debug, "Main db successfully created database table and saved queries!");
}

void mln::bot_delta::init(){
    bot.on_log(dpp::utility::cout_logger());
    bot.log(dpp::loglevel::ll_debug, is_dev_id_valid ? "Dev id found!" : "Dev id not found!");

    mln::bot_delta::setup_db();

    readys.attach_event(this);
    cmds.attach_event(this);
    ctxs.attach_event(this);
    guild_creates.attach_event(this);
}

void mln::bot_delta::initialize_environment() {
    const mln::db_result res = mln::database_handler::initialize_db_environment();
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while initializing database environment: " + mln::database_handler::get_name_from_result(res);
        throw std::exception(err_msg.c_str());
    }
}

void mln::bot_delta::shutdown_environment() {
    const mln::db_result res = mln::database_handler::shutdown_db_environment();
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred during database environment shutdown: " + mln::database_handler::get_name_from_result(res);
        throw std::exception(err_msg.c_str());
    }
}
const mln::cmd_runner& mln::bot_delta::get_cmd_runner() const {
    return cmds;
}
const mln::cmd_ctx_runner& mln::bot_delta::get_cmd_ctx_runner() const {
    return ctxs;
}


mln::bot_delta::~bot_delta(){

}

mln::bot_delta::bot_delta() :
    bot(DISCORD_BOT_TOKEN, dpp::i_default_intents | dpp::i_guild_members | dpp::i_message_content),
    dev_id(dpp::snowflake(DISCORD_DEV_ID)), 
    is_dev_id_valid(
#ifdef MLN_DB_DISCORD_DEV_ID
    true),
#else //MLN_DB_DISCORD_DEV_ID
    false),
#endif //MLN_DB_DISCORD_DEV_ID
    registered_new_cmds(false),
    db(),
    saved_select_all_query(),
    cmds(), 
    ctxs(), 
    readys(),
    guild_creates()
{
    
}

std::string mln::bot_delta::start(bool register_cmds) {
    registered_new_cmds = register_cmds;

    this->init();

    bot.start(dpp::st_return);

    return "Bot deployed!";
}

bool mln::bot_delta::close() {
    return db.close_connection() == mln::db_result::ok;
}

bool r_callback(void*) { return true; }
bool td_callback(int) { return false; }
void da_callback(void* d, int col, mln::db_column_data_t&& c_data) {
    size_t* stmt_index = static_cast<size_t*>(d);
    if (*stmt_index == 0) {
        std::cout << c_data.name << " : " << static_cast<uint64_t>(std::get<int64_t>(c_data.data)) << std::endl;
    }else {
        if (col == 0) {
            std::cout << "{ " << c_data.name << " : " << static_cast<uint64_t>(std::get<int64_t>(c_data.data));
        }else if (col == 1) {
            std::cout << "||" << c_data.name << " : " << std::get<const unsigned char*>(c_data.data);
        }else {
            std::cout << "||" << c_data.name << " : " << static_cast<uint64_t>(std::get<int64_t>(c_data.data)) << " }" << std::endl;
        }
    }
}
void si_callback(void* data, size_t stmt_i) { 
    size_t* stmt_index = static_cast<size_t*>(data);
    *stmt_index = stmt_i;
}
mln::db_result mln::bot_delta::print_main_db() const {
    bot.log(dpp::ll_debug, "Printing main db...");
    size_t stmt_index = 0;
    mln::database_callbacks_t callbacks;
    callbacks.row_callback = &r_callback;
    callbacks.type_definer_callback = &td_callback;
    callbacks.data_adder_callback = &da_callback;
    callbacks.statement_index_callback = &si_callback;
    callbacks.callback_data = static_cast<void*>(&stmt_index);
    const mln::db_result res = db.exec(saved_select_all_query, callbacks);
    bot.log(dpp::ll_debug, "Printing over.");

    return res;
}