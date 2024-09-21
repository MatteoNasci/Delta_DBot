#include "commands/ready/register_commands.h"
#include "events/cmd_runner.h"
#include "events/cmd_ctx_runner.h"
#include "utility/utility.h"
#include "utility/json_err.h"

#include <dpp/once.h>
#include <dpp/appcommand.h>
#include <dpp/cluster.h>

#include <format>

mln::register_commands::register_commands(dpp::cluster& cluster, cmd_runner& in_runner_cmd_ptr, cmd_ctx_runner& in_runner_ctx_ptr) : base_ready{ cluster }, runner_cmd_ptr{ in_runner_cmd_ptr }, runner_ctx_ptr{ in_runner_ctx_ptr } {}

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

            //mismatch can be false only if std::get<dpp::slashcommand_map>(get_commands_res.value) is valid
            add_cmds_ids_to_runners(std::get<dpp::slashcommand_map>(get_commands_res.value));

            co_return;
        }

        bot().log(dpp::loglevel::ll_debug, "Found mismatch between local commands and registered global command. Proceeding with commands registration...");

        if (!mln::utility::conf_callback_is_error(co_await bot().co_global_bulk_command_create(cmds), bot())) {
            
            bot().log(dpp::ll_debug, "Updated global commands!");

            const dpp::confirmation_callback_t get_new_commands_res = co_await bot().co_global_commands_get();
            if (get_new_commands_res.is_error()) {

                const dpp::error_info err = get_new_commands_res.get_error();
                bot().log(dpp::loglevel::ll_error, std::format("Failed to get new registered commands from API! Error: [{}], details: [{}].",
                    mln::get_json_err_text(err.code), err.human_readable));
                co_return;
            }
            if (!std::holds_alternative<dpp::slashcommand_map>(get_new_commands_res.value)) {
                bot().log(dpp::loglevel::ll_error, "Failed to get new registered commands from API! The confirmation_callback doesn't have a valid map.");
                co_return;
            }

            add_cmds_ids_to_runners(std::get<dpp::slashcommand_map>(get_new_commands_res.value));
        }
        else {
            bot().log(dpp::loglevel::ll_error, "Failed to register new commands!");
        }
    }
}

void mln::register_commands::add_cmds_ids_to_runners(const dpp::slashcommand_map& map) const
{
    runner_cmd_ptr.add_command_ids(map);
    runner_ctx_ptr.add_command_ids(map);
}
