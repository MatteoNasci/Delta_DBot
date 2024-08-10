#include "ready/register_commands.h"
#include "general/commands.h"
#include "general/ctxs.h"
#include "bot_delta.h"

#include <dpp/once.h>

dpp::task<void> mln::register_commands::command(const dpp::ready_t& event){
    if (dpp::run_once<struct register_bot_commands>()) {
        mln::bot_delta::delta().bot.log(dpp::loglevel::ll_debug, "The bot will clean and re-register the commands!");
        mln::bot_delta::delta().bot.global_bulk_command_delete();


        mln::bot_delta::delta().bot.global_bulk_command_create({
            mln::high_five::get_command(),
            mln::ping::get_command(),
            mln::bot_info::get_command(),
            mln::db::get_command(),
            mln::pm::get_command(),
            mln::msgs_get::get_command(),
            mln::add_role::get_command(),
            mln::add_emoji::get_command(),
            mln::avatar::get_command(),
            mln::help::get_command(),
            mln::report::get_command() });
    }
    co_return;
}

bool mln::register_commands::execute_command(){
	return mln::bot_delta::delta().registered_new_cmds;
}
