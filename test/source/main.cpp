


#include "bot.h"
#include <cstring>

int main(int argc, char** argv){
    bool register_bot_cmds = false;

    if(argc > 1){
        register_bot_cmds = std::strcmp(argv[1], "-r");
    }

    deploy_delta(register_bot_cmds);

    return 0;
}