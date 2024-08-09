#include "bot_delta.h"

#include "database/database_handler.h"

#include <cstring>

void run_app(const bool register_bot_cmds);


//TODO add system where someone loads a file (like cmd show), associates it with an unique name and the bot saves in in a private channel. Then when requested by another command (given the right file name from the user) show that image
//TODO add all the checks in all commands for safet, also add checks for permission of both bot and command user
//TODO https://gist.github.com/LeviSnoot/d9147767abeef2f770e9ddcd91eb85aa stuff about timestamps
int main(int argc, char** argv){
    bool register_bot_cmds = false;

    for (size_t i = 1; !register_bot_cmds && i < argc; ++i) {
        register_bot_cmds = std::strcmp(argv[i], "-r") == 0;
    }

    try {
        run_app(register_bot_cmds);
    }catch (std::exception e) {
        std::cerr << "Exception thrown! msg: " << e.what() << std::endl;
    }catch (...) {
        std::cerr << "Unknown exception thrown!" << std::endl;
    }

    return 0;
}

void run_app(const bool register_bot_cmds) {
    mln::bot_delta::initialize_environment();
    {
        mln::bot_delta delta(register_bot_cmds);

        delta.data.bot.log(dpp::loglevel::ll_info, delta.start());

        delta.data.bot.log(dpp::loglevel::ll_info, mln::database_handler::get_db_debug_info());

        delta.data.bot.log(dpp::loglevel::ll_info, "Press any key to stop the bot...");

        int c = getchar();

        delta.data.bot.log(dpp::loglevel::ll_info, "Closing the bot...");

        delta.data.bot.log(dpp::loglevel::ll_info, mln::database_handler::get_db_debug_info());

        while (!delta.close()) {
            //TODO change this while loop to something more decent
        }
    }
    mln::bot_delta::shutdown_environment();
}