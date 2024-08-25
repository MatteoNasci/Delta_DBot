#include "bot_delta.h"

#include <cstring>

void run_app(const bool register_bot_cmds);

//TODO add all the checks in all commands for safet, also add checks for permission of both bot and command user
//TODO https://gist.github.com/LeviSnoot/d9147767abeef2f770e9ddcd91eb85aa stuff about timestamps
int main(int argc, char** argv){
    bool register_bot_cmds = false;
    //TODO add better version of commands parsing below, also add different commands (one of which will be used to alter tables if needed
    for (size_t i = 1; !register_bot_cmds && i < argc; ++i) {
        register_bot_cmds = std::strcmp(argv[i], "-r") == 0;
    }

    try {
        run_app(register_bot_cmds);
    }catch (const std::exception& e) {
        std::cerr << "Exception thrown! msg: " << e.what() << std::endl;
    }catch (...) {
        std::cerr << "Unknown exception thrown!" << std::endl;
    }

    return 0;
}

void run_app(const bool register_bot_cmds) {
    mln::bot_delta::initialize_environment();
    {
        mln::bot_delta delta{};

        delta.start(register_bot_cmds);

        delta.bot.log(dpp::loglevel::ll_debug, mln::database_handler::get_db_debug_info());

        delta.bot.log(dpp::loglevel::ll_debug, "Press any key to stop the bot...");

        int c = getchar();

        delta.bot.log(dpp::loglevel::ll_debug, "Closing the bot...");

        delta.bot.log(dpp::loglevel::ll_debug, mln::database_handler::get_db_debug_info());

        delta.print_main_db();

        while (!delta.close()) {
            //TODO change this while loop to something more decent
        }
    }
    mln::bot_delta::shutdown_environment();
}