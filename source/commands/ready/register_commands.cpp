#include "commands/ready/register_commands.h"
#include "events/cmd_runner.h"
#include "events/cmd_ctx_runner.h"
#include "bot_delta.h"

#include <dpp/once.h>
#include <vector>

mln::register_commands::register_commands(mln::bot_delta* const delta) : base_ready(delta) {}

dpp::job mln::register_commands::command(std::shared_ptr<dpp::ready_t> event){
    if (dpp::run_once<struct register_bot_commands>()) {
        delta()->bot.log(dpp::loglevel::ll_debug, "The bot will clean and re-register the commands!");

        std::vector<dpp::slashcommand> cmds{};
        for (const auto& cmd_pair : delta()->get_cmd_runner().get_actions()) {
            cmds.push_back(cmd_pair.second->get_command());
        }
        for (const auto& cmd_pair : delta()->get_cmd_ctx_runner().get_actions()) {
            cmds.push_back(cmd_pair.second->get_command());
        }

        delta()->bot.global_bulk_command_create(cmds);
    }
    co_return;
}

bool mln::register_commands::execute_command(){
	return  delta()->registered_new_cmds;
}
