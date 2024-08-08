#include "bot_delta.h"

#include <cstring>

//TODO add system where someone loads a file (like cmd show), associates it with an unique name and the bot saves in in a private channel. Then when requested by another command (given the right file name from the user) show that image
//TODO add all the checks in all commands for safet, also add checks for permission of both bot and command user
//TODO https://gist.github.com/LeviSnoot/d9147767abeef2f770e9ddcd91eb85aa stuff about timestamps
int main(int argc, char** argv){
    bool register_bot_cmds = false;

    for (size_t i = 1; !register_bot_cmds && i < argc; ++i) {
        register_bot_cmds = std::strcmp(argv[i], "-r") == 0;
    }

    mln::bot_delta::initialize_environment();
    {
        mln::bot_delta delta(register_bot_cmds);

        const std::string result_text = delta.start();

        delta.data.bot.log(dpp::loglevel::ll_info, result_text);
    }
    mln::bot_delta::shutdown_environment();

    return 0;
}