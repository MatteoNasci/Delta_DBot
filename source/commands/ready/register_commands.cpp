#include "commands/ready/register_commands.h"
#include "events/cmd_runner.h"
#include "events/cmd_ctx_runner.h"
#include "bot_delta.h"

#include <dpp/once.h>
#include <vector>

mln::register_commands::register_commands(mln::bot_delta* const delta) : base_ready(delta) {}

dpp::task<void> mln::register_commands::command(const dpp::ready_t& event_data){
    if (dpp::run_once<struct register_bot_commands>()) {
        delta()->bot.log(dpp::loglevel::ll_debug, "The bot will clean and re-register the commands!");

        const mln::cmd_runner& cmd_r = delta()->get_cmd_runner();
        const mln::cmd_ctx_runner& cmd_ctx_r = delta()->get_cmd_ctx_runner();

        std::vector<dpp::slashcommand> cmds{ cmd_r.get_actions().size() + cmd_ctx_r.get_actions().size() };
        size_t i = 0;
        for (const std::pair<const std::string, std::unique_ptr<mln::base_slashcommand>>& cmd_pair : cmd_r.get_actions()) {
            cmds[i++] = cmd_pair.second->get_command();
        }
        for (const std::pair<const std::string, std::unique_ptr<mln::base_ctx_command>>& cmd_ctx_pair : cmd_ctx_r.get_actions()) {
            cmds[i++] = cmd_ctx_pair.second->get_command();
        }

        co_await delta()->bot.co_global_bulk_command_create(cmds);
    }
}

bool mln::register_commands::execute_command(){
	return delta()->registered_new_cmds;
}
