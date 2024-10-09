#include "commands/ctx/base_ctx_command.h"
#include "commands/ready/base_ready.h"
#include "commands/ready/register_commands.h"
#include "commands/slash/base_slashcommand.h"
#include "events/cmd_ctx_runner.h"
#include "events/cmd_runner.h"
#include "utility/json_err.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/misc-enum.h>
#include <dpp/once.h>
#include <dpp/restresults.h>
#include <dpp/snowflake.h>

#include <exception>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

mln::register_commands::register_commands(dpp::cluster& cluster, cmd_runner& in_runner_cmd_ptr, cmd_ctx_runner& in_runner_ctx_ptr) : base_ready{ cluster }, runner_cmd_ptr{ in_runner_cmd_ptr }, runner_ctx_ptr{ in_runner_ctx_ptr } {
    bot().log(dpp::loglevel::ll_debug, std::format("register_commands: [{}].", true));
}

dpp::task<void> mln::register_commands::command(const dpp::ready_t& event_data) const{
    if (dpp::run_once<struct register_bot_commands>()) {
        bot().log(dpp::loglevel::ll_debug, "The bot will clean and re-register the commands!");

        std::vector<dpp::slashcommand> cmds{ runner_cmd_ptr.get_actions().size() + runner_ctx_ptr.get_actions().size() };
        size_t i = 0;
        for (const std::pair<const std::string, std::unique_ptr<mln::base_slashcommand>>& cmd_pair : runner_cmd_ptr.get_actions()) {
            cmds[i++] = cmd_pair.second->get_cmd();
        }
        for (const std::pair<const std::string, std::unique_ptr<mln::base_ctx_command>>& cmd_ctx_pair : runner_ctx_ptr.get_actions()) {
            cmds[i++] = cmd_ctx_pair.second->get_cmd();
        }

        //Ugly double cycle is only performed once
        for (size_t i = 0; i < cmds.size(); ++i) {
            for (size_t j = i + 1; j < cmds.size(); ++j) {
                if (mln::utility::is_same_cmd(cmds[i], cmds[j])) {
                    bot().log(dpp::loglevel::ll_error, std::format("Failed to register commands, found duplicate local commands! Command name: [{}], indexes: [{}, {}].", 
                        cmds[i].name, i, j));
                    co_return;
                }
            }
        }

        const dpp::confirmation_callback_t get_commands_res = co_await bot().co_global_commands_get();

        bool mismatch = false;

        if (get_commands_res.is_error()) {

            mismatch = true;

            const dpp::error_info err = get_commands_res.get_error();
            bot().log(dpp::loglevel::ll_error, std::format("Failed to get registered commands from API! Error: [{}], details: [{}].\nProceeding with commands registration...", 
                mln::get_json_err_text(err.code), err.human_readable));
        }
        else if (!std::holds_alternative<dpp::slashcommand_map>(get_commands_res.value)) {

            mismatch = true;

            bot().log(dpp::loglevel::ll_error, "Failed to get registered commands from API! The confirmation_callback doesn't have a valid map. Proceeding with commands registration...");
        } 
        else{

            mismatch = std::get<dpp::slashcommand_map>(get_commands_res.value).size() != cmds.size();

            if (!mismatch) {

                //Not efficient double loop but it's fine, only executed once in whole program lifetime
                for (const dpp::slashcommand& cmd : cmds) {
                    bool found = false;
                    for (const std::pair<dpp::snowflake, dpp::slashcommand>& cmd_pair : std::get<dpp::slashcommand_map>(get_commands_res.value)) {
                        if (mln::utility::is_same_cmd(cmd, cmd_pair.second)) {
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        mismatch = true;
                        break;
                    }
                }
            }
        }

        if (!mismatch) {
            bot().log(dpp::loglevel::ll_debug, "Found no mismatch between local commands and registered global command. Commands registration skipped!");

            co_return;
        }

        bot().log(dpp::loglevel::ll_debug, "Found mismatch between local commands and registered global command. Proceeding with commands registration...");

        const dpp::confirmation_callback_t register_res = co_await bot().co_global_bulk_command_create(cmds);
        if (!register_res.is_error()) {
            bot().log(dpp::ll_debug, "Updated global commands!");
        }
        else {
            const dpp::error_info err = register_res.get_error();
            bot().log(dpp::loglevel::ll_error, std::format("Failed to register new commands! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable));
            
            throw std::exception("Failed to register new commands!");
        }
    }
}

std::optional<std::function<void()>> mln::register_commands::job(const dpp::ready_t& event_data) const
{
    bot().log(dpp::loglevel::ll_critical, "Failed to register commands! Usage of job command instead of task command.");
    return std::nullopt;
}

bool mln::register_commands::use_job() const
{
    return false;
}
