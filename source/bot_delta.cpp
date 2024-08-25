#include "bot_delta.h"
#include "defines.h"
#include "utility/constants.h"
#include "utility/utility.h"

static const std::string s_execute_pragmas("PRAGMA foreign_keys = ON;PRAGMA optimize=0x10002;"); //TODO also make "PRAGMA optimize;" run either once per day or when the db has changed a lot (maybe even more frequently than 1 day)
static const std::string s_create_main_table("CREATE TABLE IF NOT EXISTS guild_profile( guild_id INTEGER PRIMARY KEY NOT NULL, dedicated_channel_id INTEGER NOT NULL DEFAULT(0));"); //TODO use WAL sqlite
static const std::string s_create_report_table("CREATE TABLE IF NOT EXISTS report( id INTEGER PRIMARY KEY, guild_id INTEGER NOT NULL, user_id INTEGER NOT NULL, report_text TEXT NOT NULL, creation_time TEXT NOT NULL DEFAULT (datetime('now')));");
//primary key should be both the foreign key and 'name', to allow different guilds to use same name
static const std::string s_create_storage_table("CREATE TABLE IF NOT EXISTS storage( guild_id INTEGER NOT NULL REFERENCES guild_profile(guild_id), name TEXT NOT NULL CHECK (LENGTH(name) >= " 
    + std::to_string(static_cast<int64_t>(mln::bot_delta::min_text_id_size())) + " AND LENGTH(name) <= "
    + std::to_string(static_cast<int64_t>(mln::bot_delta::max_text_id_size())) + "), url TEXT NOT NULL, desc TEXT DEFAULT (NULL), user_id INTEGER NOT NULL, creation_time TEXT NOT NULL DEFAULT (datetime('now')), PRIMARY KEY(guild_id, name));");

static const std::string s_select_dump_channel("SELECT dedicated_channel_id FROM guild_profile WHERE guild_id = ?;");
static const std::string s_select_all("SELECT * FROM guild_profile; SELECT * FROM storage ORDER BY guild_id ASC, user_id ASC, creation_time ASC; SELECT * FROM report ORDER BY guild_id ASC, user_id ASC, creation_time ASC;");
static const std::string s_select_all_guild_profile("SELECT guild_id, dedicated_channel_id FROM guild_profile;");

size_t mln::bot_delta::max_text_id_size() {
    return 50;
}
size_t mln::bot_delta::min_text_id_size() {
    return 1;
}
//TODO once the bot db gets big it might be usefull to use a cache instead of always interrogating the database, same for retrieved text messages (where i have to do an api call to get that single message for its contents)
void mln::bot_delta::setup_db() {
    mln::db_result res = db.open_connection("dbs/main.db");
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while connecting to database: " + mln::database_handler::get_name_from_result(res) + ". " + db.get_last_err_msg();
        throw std::exception(err_msg.c_str());
    }

    res = db.exec(s_execute_pragmas, mln::database_callbacks_t());
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while executing pragmas: " + mln::database_handler::get_name_from_result(res) + ". " + db.get_last_err_msg();
        throw std::exception(err_msg.c_str());
    }

    res = db.exec(s_create_main_table, mln::database_callbacks_t());
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while creating the guild_profile table: " + mln::database_handler::get_name_from_result(res) + ". " + db.get_last_err_msg();
        throw std::exception(err_msg.c_str());
    }

    res = db.exec(s_create_report_table, mln::database_callbacks_t());
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while creating the report table: " + mln::database_handler::get_name_from_result(res) + ". " + db.get_last_err_msg();
        throw std::exception(err_msg.c_str());
    }

    res = db.exec(s_create_storage_table, mln::database_callbacks_t());
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while creating the storage table: " + mln::database_handler::get_name_from_result(res) + ". " + db.get_last_err_msg();
        throw std::exception(err_msg.c_str());
    }

    res = db.save_statement(s_select_all, saved_select_all_query);
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while saving the select all stmt: " + mln::database_handler::get_name_from_result(res) + ". " + db.get_last_err_msg();
        throw std::exception(err_msg.c_str());
    }

    res = db.save_statement(s_select_all_guild_profile, saved_select_all_gp_query);
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while saving the select all guild profile stmt: " + mln::database_handler::get_name_from_result(res) + ". " + db.get_last_err_msg();
        throw std::exception(err_msg.c_str());
    }

    res = db.save_statement(s_select_dump_channel, saved_select_dump_channel);
    if (res != mln::db_result::ok) {
        std::string err_msg = "An error occurred while saving the select dump channel stmt: " + mln::database_handler::get_name_from_result(res) + ". " + db.get_last_err_msg();
        throw std::exception(err_msg.c_str());
    }

    bot.log(dpp::loglevel::ll_debug, "Main db successfully created database table and saved queries!");
}

void mln::bot_delta::init(){
    bot.on_log(dpp::utility::cout_logger());
    bot.log(dpp::loglevel::ll_debug, is_dev_id_valid ? "Dev id found!" : "Dev id not found!");

    mln::bot_delta::setup_db();

    if (recalculate_dump_channels_cache() != mln::db_result::ok) {
        throw std::exception("An error occurred while creating guild_profile cache!");
    }

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
    bot(DISCORD_BOT_TOKEN, dpp::i_default_intents | dpp::i_guild_members | dpp::i_message_content, 0, 0, 1, true,
        dpp::cache_policy_t{
            .user_policy = dpp::cache_policy_setting_t::cp_none,
            .emoji_policy = dpp::cache_policy_setting_t::cp_none,
            .role_policy = dpp::cache_policy_setting_t::cp_none, 
            .channel_policy = dpp::cache_policy_setting_t::cp_none,
            .guild_policy = dpp::cache_policy_setting_t::cp_none,
             }),
    dev_id(dpp::snowflake(DISCORD_DEV_ID)),
    is_dev_id_valid(
    #ifdef MLN_DB_DISCORD_DEV_ID
        true),
#else //MLN_DB_DISCORD_DEV_ID
    false),
#endif //MLN_DB_DISCORD_DEV_ID
    registered_new_cmds(false),
    db(),
    dump_channels_cache(),
    messages_cache(),
    show_all_cache(),
    show_user_cache(),
    guild_cache(),
    channel_cache(),
    saved_select_all_query(),
    saved_select_all_gp_query(),
    saved_select_dump_channel(),
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
//TODO manage intents permissions as well
bool mln::bot_delta::close() {
    return db.close_connection() == mln::db_result::ok;
}
//TODO separate report table into it's own db file
bool td_callback(void* d, int c) { 
    size_t* stmt_index = static_cast<size_t*>(d);
    return (*stmt_index == 1 && (c == 1 || c == 2 || c == 3 || c == 5)) || (*stmt_index == 2 && (c == 3 || c == 4)); //TODO use state machine where I switch function instead of this shit, same for below (in class interpreter since i'm interpreting the data)
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
            std::cout << " || " << c_data.name << " : " << std::get<const unsigned char*>(c_data.data) << " }" << std::endl;
        }
    }else if(*stmt_index == 2){
        if (col == 0) {
            std::cout << "{ " << c_data.name << " : " << static_cast<uint64_t>(std::get<int64_t>(c_data.data));
        }else if (col == 1) {
            std::cout << " || " << c_data.name << " : " << static_cast<uint64_t>(std::get<int64_t>(c_data.data));
        }else if (col == 2) {
            std::cout << " || " << c_data.name << " : " << static_cast<uint64_t>(std::get<int64_t>(c_data.data));
        }else if (col == 3) {
            std::cout << " || " << c_data.name << " : " << std::get<const unsigned char*>(c_data.data);
        }else if (col == 4) {
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
mln::db_result mln::bot_delta::print_main_db() const {
    bot.log(dpp::ll_debug, "Printing main db...");

    size_t stmt_index = 0;
    mln::database_callbacks_t callbacks{};
    callbacks.type_definer_callback = &td_callback;
    callbacks.data_adder_callback = &da_callback;
    callbacks.statement_index_callback = &si_callback;
    callbacks.callback_data = static_cast<void*>(&stmt_index);
    const mln::db_result res = db.exec(saved_select_all_query, callbacks);

    bot.log(dpp::ll_debug, "Printing over.");

    return res;
}

bool td_gp_callback(void*, int) {
    return false;
}
void da_gp_callback(void* d, int col, mln::db_column_data_t&& c_data) {
    mln::bot_delta::upd_dump_channel_t* map_ptr = static_cast<mln::bot_delta::upd_dump_channel_t*>(d);

    if (col == 0) {
        map_ptr->guild = static_cast<uint64_t>(std::get<int64_t>(c_data.data));
        return;
    }

    const bool is_channel_null = std::holds_alternative<const short*>(c_data.data);
    map_ptr->cache.add_element(map_ptr->guild, is_channel_null ? 0 : static_cast<uint64_t>(std::get<int64_t>(c_data.data)));
}

mln::db_result mln::bot_delta::recalculate_dump_channels_cache() {

    dump_channels_cache.clear();
    mln::bot_delta::upd_dump_channel_t data{.cache = dump_channels_cache, .guild = 0};
    mln::database_callbacks_t callbacks{};
    callbacks.type_definer_callback = &td_gp_callback;
    callbacks.data_adder_callback = &da_gp_callback;
    callbacks.callback_data = static_cast<void*>(&data);
    
    return db.exec(saved_select_all_gp_query, callbacks);
}
//TODO check end of dpp/snowflake.h file, see if i can use the hash trick
bool td_dc_callback(void*, int) {
    return false;
}
void da_dc_callback(void* d, int col, mln::db_column_data_t&& c_data) {
    std::optional<uint64_t>* opt_ptr = static_cast<std::optional<uint64_t>*>(d);

    *opt_ptr = static_cast<uint64_t>(std::get<int64_t>(c_data.data));
}

std::optional<uint64_t> mln::bot_delta::get_db_dump_channel(uint64_t guild_id) {
    std::optional<uint64_t> result{std::nullopt};
    db_result res = db.bind_parameter(saved_select_dump_channel, 0, 1, static_cast<int64_t>(guild_id));
    if (res != mln::db_result::ok) {
        bot.log(dpp::loglevel::ll_error, "Failed to bind query parameters for select dump channel! Error: " + mln::database_handler::get_name_from_result(res) + ", " + db.get_last_err_msg());
        return result;
    }

    mln::database_callbacks_t calls{};
    calls.callback_data = static_cast<void*>(&result);
    calls.type_definer_callback = &td_dc_callback;
    calls.data_adder_callback = &da_dc_callback;

    db.exec(saved_select_dump_channel, calls);
    if (res != mln::db_result::ok) {
        bot.log(dpp::loglevel::ll_error, "Failed to execute query for select dump channel! Error: " + mln::database_handler::get_name_from_result(res) + ", " + db.get_last_err_msg());
    }

    if (result.has_value()) {
        dump_channels_cache.add_element(guild_id, result.value());
    }

    return result;
}
