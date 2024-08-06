#include "ready/register_commands.h"
#include "general/commands.h"
#include "general/ctxs.h"

#include <dpp/once.h>

dpp::task<void> mln::register_commands::command(mln::bot_delta_data_t& data, const dpp::ready_t& event_data){
    if (dpp::run_once<struct register_bot_commands>()) {
        data.bot.log(dpp::loglevel::ll_debug, "The bot will clean and re-register the commands!");
        data.bot.global_bulk_command_delete();


        data.bot.global_bulk_command_create({
            mln::high_five::get_command(data.bot),
            mln::ping::get_command(data.bot),
            mln::bot_info::get_command(data.bot),
            mln::db::get_command(data.bot),
            mln::pm::get_command(data.bot),
            mln::msgs_get::get_command(data.bot),
            mln::add_role::get_command(data.bot),
            mln::add_emoji::get_command(data.bot),
            mln::avatar::get_command(data.bot),
            mln::help::get_command(data.bot),
            mln::report::get_command(data.bot) });
    }
    co_return;
}

bool mln::register_commands::execute_command(mln::bot_delta_data_t& data){
	return data.registered_new_cmds;
}
