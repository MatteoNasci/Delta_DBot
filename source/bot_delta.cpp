#include "mln.h"
#include "defines.h"
#include <dpp/intents.h>
#include "bot_delta.h"
#include <dpp/dpp.h>

bot_delta_data_t::bot_delta_data_t(dpp::snowflake in_dev_id, bool in_is_dev_id_valid) : bot(DISCORD_BOT_TOKEN, dpp::i_default_intents | dpp::i_guild_members | dpp::i_message_content), dev_id(in_dev_id), is_dev_id_valid(in_is_dev_id_valid)
{

}

void bot_delta::init(const bool register_cmds)
{
    data.bot.on_log(dpp::utility::cout_logger());
    data.bot.log(dpp::loglevel::ll_info, data.is_dev_id_valid ? "Dev id found!" : "Dev id not found!");

    /*Note
    Once your bot gets big, it's not recommended to create slash commands in the on_ready event even when it's inside dpp::run_once as, 
    if you re-run your bot multiple times or start multiple clusters, you will quickly get rate-limited! 
    You could, for example, add a commandline parameter to your bot (argc, argv) so that if you want the bot to register commands it must be launched with a specific command line argument.*/
    data.bot.on_ready([this, register_cmds](const dpp::ready_t& event) {
        if(!register_cmds){
            return;
        }

        if (dpp::run_once<struct register_bot_commands>()) {  
            data.bot.global_bulk_command_delete();
                
            
            data.bot.global_bulk_command_create({ 
                high_five::get_command(data.bot),
                ping::get_command(data.bot),
                pong::get_command(data.bot),
                thinking::get_command(data.bot),
                bot_info::get_command(data.bot),
                file::get_command(data.bot),
                pm::get_command(data.bot),
                msgs_get::get_command(data.bot), 
                channel_create::get_command(data.bot), 
                msg_error::get_command(data.bot),
                image::get_command(data.bot),
                blep::get_command(data.bot),
                show::get_command(data.bot),
                add_role::get_command(data.bot),
                button::get_command(data.bot),
                button2::get_command(data.bot),
                math::get_command(data.bot),
                pring::get_command(data.bot),
                select::get_command(data.bot),
                select2::get_command(data.bot),
                select3::get_command(data.bot),
                dialog::get_command(data.bot) });

            }
    });

    cmds.init(data);
    ctxs.init(data);
    forms.init(data);
    selects.init(data);
}


bot_delta::bot_delta(const bool register_cmds) : 
    data( 
    dpp::snowflake(DISCORD_DEV_ID), 
#ifdef MLN_DB_DISCORD_DEV_ID
    true
#else
    false
#endif
    ),
    cmds(), 
    ctxs(), 
    forms(), 
    selects()
{
    this->init(register_cmds);
}

std::string bot_delta::start()
{
    data.bot.start(dpp::st_wait);
    return "Bot closed!";
}
