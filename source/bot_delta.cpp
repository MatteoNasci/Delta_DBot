#include "bot_delta.h"
#include "defines.h"
#include "utility/constants.h"
#include "utility/utility.h"
#include "utility/caches.h"

#include <iostream>
#include <cstdio>
#include <format>

size_t mln::bot_delta::max_text_id_size() {
    return 50;
}
size_t mln::bot_delta::min_text_id_size() {
    return 1;
}

void mln::bot_delta::setup_db() {
    mln::db_result_t res = db.open_connection("dbs/main.db");
    if (res.type != mln::db_result::ok) {
        const std::string err_msg = std::format("An error occurred while connecting to database. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
        throw std::exception(err_msg.c_str());
    }

    res = db.exec("PRAGMA foreign_keys = ON;PRAGMA optimize=0x10002;", mln::database_callbacks_t());
    if (mln::database_handler::is_exec_error(res.type)) {
        const std::string err_msg = std::format("An error occurred while executing pragmas. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
        throw std::exception(err_msg.c_str());
    }

    res = db.exec("CREATE TABLE IF NOT EXISTS guild_profile( guild_id INTEGER PRIMARY KEY NOT NULL, dedicated_channel_id INTEGER NOT NULL DEFAULT(0));", mln::database_callbacks_t());
    if (mln::database_handler::is_exec_error(res.type)) {
        const std::string err_msg = std::format("An error occurred while creating the guild_profile table. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
        throw std::exception(err_msg.c_str());
    }

    res = db.exec("CREATE TABLE IF NOT EXISTS report( id INTEGER PRIMARY KEY, report_text TEXT NOT NULL, creation_time TEXT NOT NULL DEFAULT (datetime('now')));", mln::database_callbacks_t());
    if (mln::database_handler::is_exec_error(res.type)) {
        const std::string err_msg = std::format("An error occurred while creating the report table. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
        throw std::exception(err_msg.c_str());
    }
    //primary key should be both the foreign key and 'name', to allow different guilds to use same name. Make sure max/min_text_id_size are synced with the statement below
    res = db.exec("CREATE TABLE IF NOT EXISTS storage( guild_id INTEGER NOT NULL REFERENCES guild_profile(guild_id), name TEXT NOT NULL CHECK (LENGTH(name) >= "
        "1" " AND LENGTH(name) <= "
        "50" "), url TEXT NOT NULL, desc TEXT DEFAULT (NULL), user_id INTEGER NOT NULL, nsfw INTEGER NOT NULL DEFAULT (0), creation_time TEXT NOT NULL DEFAULT (datetime('now')), PRIMARY KEY(guild_id, name));", mln::database_callbacks_t());
    if (mln::database_handler::is_exec_error(res.type)) {
        const std::string err_msg = std::format("An error occurred while creating the storage table. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
        throw std::exception(err_msg.c_str());
    }

    res = db.save_statement("SELECT * FROM guild_profile; SELECT * FROM storage ORDER BY guild_id ASC, user_id ASC, creation_time ASC; SELECT * FROM report ORDER BY creation_time ASC;", saved_select_all_query);
    if (res.type != mln::db_result::ok) {
        const std::string err_msg = std::format("An error occurred saving the select all stmt. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
        throw std::exception(err_msg.c_str());
    }

    res = db.save_statement("SELECT guild_id, dedicated_channel_id FROM guild_profile;", saved_select_all_gp_query);
    if (res.type != mln::db_result::ok) {
        const std::string err_msg = std::format("An error occurred while saving the select all guild_profile stmt. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
        throw std::exception(err_msg.c_str());
    }

    res = db.save_statement("PRAGMA optimize;", saved_optimize_db);
    if (res.type != mln::db_result::ok) {
        const std::string err_msg = std::format("An error occurred while saving the optimize db stmt. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
        throw std::exception(err_msg.c_str());
    }

    static constexpr uint64_t s_optimize_timer_seconds{ 60 * 60 * 24 };
    db_optimize_timer = bot.start_timer([this](dpp::timer timer) {
        bot.log(dpp::loglevel::ll_debug, "Optimizing database...");
        const mln::db_result_t res = db.exec(saved_optimize_db, mln::database_callbacks_t());
        if (mln::database_handler::is_exec_error(res.type)) {
            bot.log(dpp::loglevel::ll_error, std::format("An error occurred while optimizing database. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text));
        }
        else {
            bot.log(dpp::loglevel::ll_debug, "Database optimized!");
        }
        }, s_optimize_timer_seconds);

    bot.log(dpp::loglevel::ll_debug, "Main db successfully created, saved queries!");
}

void mln::bot_delta::logger(const dpp::log_t& log)
{
    static const std::function<void(const dpp::log_t&)> default_logger = dpp::utility::cout_logger();
    switch (log.severity) {
    case dpp::loglevel::ll_trace:
        break;
    case dpp::ll_debug:
    case dpp::loglevel::ll_info:
        default_logger(log);
        break;
    default:
        default_logger(log);
        mln::bot_delta::log_to_file(log);
        break;
    }
}

void mln::bot_delta::log_to_file(const dpp::log_t& log)
{
    static constexpr std::string s_severity_map[] = {"Trace", "Debug", "Info", "Warning", "Error", "Critical"};

    std::clog << "{ [" << mln::utility::get_current_date_time() << " " << s_severity_map[log.severity] << "] " << log.message << " }\n";
}

void mln::bot_delta::init(){
    log_file = std::freopen("log.txt", "a", stderr);
    if (!log_file) {
        throw std::exception("Failed to open log file!");
    }
    bot.on_log(mln::bot_delta::logger);

    bot.log(dpp::loglevel::ll_critical, "Starting new bot session...");
    bot.log(dpp::loglevel::ll_debug, is_dev_id_valid ? "Dev id found!" : "Dev id not found!");

    mln::bot_delta::setup_db();

    readys.attach_event();
    cmds.attach_event();
    ctxs.attach_event();
    guild_creates.attach_event();

    bot.log(dpp::loglevel::ll_debug, "Events attached!");

    mln::caches::init(&bot, &db);

    bot.log(dpp::loglevel::ll_debug, "Caches initialized!");
}

void mln::bot_delta::initialize_environment() {
    const mln::db_result_t res = mln::database_handler::initialize_db_environment();
    if (res.type != mln::db_result::ok) {
        const std::string err_msg = std::format("An error occurred while initializing database environment. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
        throw std::exception(err_msg.c_str());
    }
}

void mln::bot_delta::shutdown_environment() {
    const mln::db_result_t res = mln::database_handler::shutdown_db_environment();
    if (res.type != mln::db_result::ok) {
        const std::string err_msg = std::format("An error occurred while database environment shutdown. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
        throw std::exception(err_msg.c_str());
    }
}
const mln::cmd_runner& mln::bot_delta::get_cmd_runner() const {
    return cmds;
}
const mln::cmd_ctx_runner& mln::bot_delta::get_cmd_ctx_runner() const {
    return ctxs;
}

const bool mln::bot_delta::is_bot_running() const
{
    return running;
}

mln::bot_delta::~bot_delta(){
    close();
}

mln::bot_delta::bot_delta() :
    bot{ DISCORD_BOT_TOKEN, dpp::i_default_intents | dpp::i_guild_members | dpp::i_message_content, 0, 0, 1, true,
        dpp::cache_policy_t{
            .user_policy = dpp::cache_policy_setting_t::cp_none,
            .emoji_policy = dpp::cache_policy_setting_t::cp_none,
            .role_policy = dpp::cache_policy_setting_t::cp_none,
            .channel_policy = dpp::cache_policy_setting_t::cp_none,
            .guild_policy = dpp::cache_policy_setting_t::cp_none,
        } },
        dev_id{ dpp::snowflake{DISCORD_DEV_ID} },
        is_dev_id_valid{
#ifdef MLN_DB_DISCORD_DEV_ID
            true},
#else //MLN_DB_DISCORD_DEV_ID
    false},
#endif //MLN_DB_DISCORD_DEV_ID
    db{},
    db_optimize_timer{},
    saved_optimize_db{},
    saved_select_all_query{},
    saved_select_all_gp_query{},
    cmds{bot, db},
    ctxs{bot, db},
    readys{bot, db, cmds, ctxs},
    guild_creates{bot, db},
    running{ false },
    log_file{ nullptr }
{
    
}

std::string mln::bot_delta::start() {
    if (!is_bot_running()) {
        this->init();

        running = true;
        bot.start(dpp::st_return);

        return "Bot deployed!";
    }
    return "Bot is already running!";
}
bool mln::bot_delta::close() {
    if (is_bot_running()) {
        bot.stop_timer(db_optimize_timer);

        bot.shutdown();
        running = false;
    }

    if (log_file) {
        std::fclose(log_file);
        log_file = nullptr;
    }

    mln::caches::cleanup();

    return db.close_connection().type == mln::db_result::ok;
}
bool td_callback(void* d, int c) { 
    size_t* stmt_index = static_cast<size_t*>(d);
    return (*stmt_index == 1 && (c == 1 || c == 2 || c == 3 || c == 5 || c == 6)) || (*stmt_index == 2 && (c == 1 || c == 2));
}
void da_callback(void* d, int col, mln::db_column_data_t&& c_data) {
    size_t* stmt_index = static_cast<size_t*>(d);
    if (*stmt_index == 0) {
        col == 0 ? std::cout << "{ " << c_data.name << " : " << static_cast<uint64_t>(std::get<int64_t>(c_data.data)) : 
             std::cout << " || " << c_data.name << " : " << static_cast<uint64_t>(std::get<int64_t>(c_data.data)) << " }" << std::endl;
    }else if(*stmt_index == 1) {
        if (col == 0) {
            std::cout << "{ " << c_data.name << " : " << static_cast<uint64_t>(std::get<int64_t>(c_data.data));
        }else if (col == 1) {
            std::cout << " || " << c_data.name << " : " << std::get<const unsigned char*>(c_data.data);
        }else if (col == 2){
            std::cout << " || " << c_data.name << " : " << std::get<const unsigned char*>(c_data.data);
        }else if (col == 3) {
            std::cout << " || " << c_data.name << " : " << (std::holds_alternative<const short*>(c_data.data) ? "NULL" : reinterpret_cast<const char*>(std::get<const unsigned char*>(c_data.data)));
        }else if (col == 4) {
            std::cout << " || " << c_data.name << " : " << static_cast<uint64_t>(std::get<int64_t>(c_data.data));
        }else if (col == 5) {
            std::cout << " || " << c_data.name << " : " << static_cast<bool>(std::get<int>(c_data.data));
        }else if (col == 6) {
            std::cout << " || " << c_data.name << " : " << std::get<const unsigned char*>(c_data.data) << " }" << std::endl;
        }
    }else if(*stmt_index == 2){
        if (col == 0) {
            std::cout << "{ " << c_data.name << " : " << static_cast<uint64_t>(std::get<int64_t>(c_data.data));
        }else if (col == 1) {
            std::cout << " || " << c_data.name << " : " << std::get<const unsigned char*>(c_data.data);
        }else if (col == 2) {
            std::cout << " || " << c_data.name << " : " << std::get<const unsigned char*>(c_data.data) << " }" << std::endl;
        }
    }
}
void si_callback(void* data, size_t stmt_i) { 
    size_t* stmt_index = static_cast<size_t*>(data);
    *stmt_index = stmt_i;
    if (*stmt_index == 0) {
        std::cout << "****guild_profile table****" << std::endl;
    }else if (*stmt_index == 1) {
        std::cout << "****storage table****" << std::endl;
    } else if (*stmt_index == 2){
        std::cout << "****report table****" << std::endl;
    }
}
mln::db_result_t mln::bot_delta::print_main_db() const {
    bot.log(dpp::ll_debug, "Printing main db...");

    size_t stmt_index = 0;
    mln::database_callbacks_t callbacks{};
    callbacks.type_definer_callback = &td_callback;
    callbacks.data_adder_callback = &da_callback;
    callbacks.statement_index_callback = &si_callback;
    callbacks.callback_data = static_cast<void*>(&stmt_index);
    const mln::db_result_t res = db.exec(saved_select_all_query, callbacks);

    bot.log(dpp::ll_debug, "Printing over.");

    return res;
}