


#include "bot_delta.h"
#include <cstring>

//TODO add system where someone loads a file (like cmd show), associates it with an unique name and the bot saves in in a private channel. Then when requested by another command (given the right file name from the user) show that image
int main(int argc, char** argv){
    bool register_bot_cmds = false;

    for (size_t i = 1; !register_bot_cmds && i < argc; ++i) {
        register_bot_cmds = std::strcmp(argv[i], "-r") == 0;
    }

    mln::bot_delta delta(register_bot_cmds);

    const std::string result_text = delta.start();

    delta.data.bot.log(dpp::loglevel::ll_info, result_text);

    return 0;
}