


#include "bot.h"
#include <cstring>

int main(int argc, char** argv){
    bool register_bot_cmds = false;

    for (size_t i = 1; i < argc; ++i) {
        if (std::strcmp(argv[1], "-r") == 0) {
            register_bot_cmds = true;
            break;
        }
    }

    deploy_delta(register_bot_cmds);

    return 0;
}