#include "bot_delta.h"
#include "defines.h"
#include "database/database_handler.h"

#include <dpp/dpp.h>
#include <dpp/intents.h>

mln::bot_delta_data_t::bot_delta_data_t(dpp::snowflake in_dev_id, bool in_is_dev_id_valid, bool in_register_commands) :
    bot(DISCORD_BOT_TOKEN, dpp::i_default_intents | dpp::i_guild_members | dpp::i_message_content), 
    dev_id(in_dev_id), 
    is_dev_id_valid(in_is_dev_id_valid),
    registered_new_cmds(in_register_commands){}

void mln::bot_delta::init(){
    data.bot.on_log(dpp::utility::cout_logger());
    data.bot.log(dpp::loglevel::ll_info, data.is_dev_id_valid ? "Dev id found!" : "Dev id not found!");

    const mln::db_result res = db.open_connection("data.db");
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        data.bot.log(dpp::loglevel::ll_critical, "An error occurred while connecting to database: " + err_msg);
        throw std::exception("An error occurred while connecting to database");
    }
    
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
        std::cerr << "An error occurred while initializing database environment: " << err_msg << std::endl;
        throw std::exception("An error occurred while initializing database configs");
    }
}

void mln::bot_delta::shutdown_environment(){
    const mln::db_result res = mln::database_handler::shutdown_db_environment();
    if (res != mln::db_result::ok) {
        std::string err_msg("Error name not found");
        mln::database_handler::get_name_from_result(res, err_msg);
        std::cerr << "An error occurred during database environment shutdown: " << err_msg << std::endl;
        throw std::exception("An error occurred during database environment shutdown");
    }
}


mln::bot_delta::~bot_delta(){
    
}

mln::bot_delta::bot_delta(const bool register_cmds) :
    data( 
    dpp::snowflake(DISCORD_DEV_ID), 
#ifdef MLN_DB_DISCORD_DEV_ID
    true,
#else //MLN_DB_DISCORD_DEV_ID
    false,
#endif //MLN_DB_DISCORD_DEV_ID
    register_cmds),
    db(),
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
