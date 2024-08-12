#include "commands/autocomplete/base_autocomplete.h"

mln::base_autocomplete::base_autocomplete(const std::string& cmd_name) : base_action(cmd_name){}

dpp::task<void> mln::base_autocomplete::command(const dpp::autocomplete_t& event, const dpp::command_option& opt){
    co_return;
}
