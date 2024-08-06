#include "mln.h"
#include "defines.h"
#include "bot_delta.h"

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


mln::bot_delta::bot_delta(const bool register_cmds) :
    data( 
    dpp::snowflake(DISCORD_DEV_ID), 
#ifdef MLN_DB_DISCORD_DEV_ID
    true,
#else //MLN_DB_DISCORD_DEV_ID
    false,
#endif //MLN_DB_DISCORD_DEV_ID
    register_cmds),
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
    data.bot.start(dpp::st_wait);
    return "Bot closed!";
}
