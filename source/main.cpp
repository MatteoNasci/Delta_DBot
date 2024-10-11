#include "bot_delta.h"
#include "database/database_handler.h"
#include "utility/caches.h"
#include "utility/logs.h"
#include "utility/utility.h"

#include <dpp/misc-enum.h>

#include <cstdio>
#include <exception>
#include <format>
#include <chrono>
#include <thread>

bool run_app();

//TODO sqlite is not really adeguate for a discord bot (if the bot is big there will be a lot of concurrency problems). When needed change to something else (maybe mysql?)
//TODO add all the checks in all commands for safet, also add checks for permission of both bot and command user
//TODO https://gist.github.com/LeviSnoot/d9147767abeef2f770e9ddcd91eb85aa stuff about timestamps
//TODO add better version of commands parsing in the main (both initial cmd parsing and cmd parsing while executing), also add different commands (one of which will be used to alter tables if needed)
//TODO change main.cpp close while loop to something more decent
//TODO utility/cache.h the multimap is the bottleneck of this class, maybe using a priority queue would be better? Or I could use a std::map<size_t, std::unordered_set<T_key>> which will mantaint the size_t ordering
//TODO use WAL sqlite
//TODO manage intents permissions as well
//TODO use state machine where I switch function instead of this shit in the print db (in class interpreter since i'm interpreting the data)
//TODO add_role check if the user/bot actually can give the selected role to the target (more checks needed probably)
//TODO verify if I need permissions for responding to interaction (like send_msg, attach file). Answer probably yes
//TODO I need to add encryption for user data, other security measures and inform user of data stored. Maybe use libsodium, already added to cmakelists. Use AWS keys to store the decryption key.
//TODO add automatic db backups
//TODO https://support-dev.discord.com/hc/en-us/articles/8562894815383-Discord-Developer-Terms-of-Service section 5, add /db privacy policy command to explain data usage/storage and deletion
//TODO https://gdpr.eu/
//TODO change thinking response to something else that warns user the wait might be long (for select and insert)
//TODO add limits to api requests/uploads/ram usage
//TODO collect http_info from all confirmation_callbacks
//TODO utility maybe replace regex? It's slow but it is not a necessity for it to be fast
//TODO make static stuff constepr where possible
//TODO remove secrets from files, maybe use env vars
//TODO TODO Make separate thread for db manipulation and other slow stuff. Probably one or more queues to one thread. Uno queue per event_handler, the base_action will have additional funcs, one to tell event_handler if we want to execute on current thread or other thread, then another command func to give to the queue
//TODO add statistics and time stuff
//TODO change exceptions with proper class esceptions
//TODO when doing .is_exception(), use a try cacth to retrieve exception and display msg

//TODO remove as much static as possible
//TODO use chronometer to time stuff

/*
To test: fare qui lista di cose da testare. Ad ogni singolo test associare i relativi comandi discord

all /help commands, including /db privacy policy, /info and /changelog. They all need to display text info as ephermeral reply to the command.

For /db config:
Test /db update_dump_channel with valid channel, test inserts end up in dump channel, test select works properly
Test /db update_dump_channel with same channel again, test inserts end up in dump channel, test select works properly
Test /db update_dump_channel with no channel, test inserts end up in local channel, test select works properly
Test /db update_dump_channel with no channel again, test inserts end up in local channel, test select works properly
.
.
.
*/
int main(int argc, char** argv){
    mln::logs logger{"log.txt"};

    mln::logs::log_to_file_and_terminal(dpp::loglevel::ll_critical, std::format("Is dev build? [{}]", mln::utility::is_dev_build()));

    for (size_t i = 1; i < argc; ++i) {

    }

    for (;;) {
        bool is_error = true;
        try {
            is_error = run_app();
        }
        catch (const std::exception& e) {
            mln::logs::log_to_file_and_terminal(dpp::loglevel::ll_critical, std::format("Exception thrown! msg: [{}].", e.what()));
        }
        catch (...) {
            mln::logs::log_to_file_and_terminal(dpp::loglevel::ll_critical, "Exception thrown! Unknown msg.");
        }

        if (!is_error) {
            break;
        }

        mln::bot_delta::shutdown_environment();

        static const size_t s_delay = 5;
        mln::logs::log_to_file_and_terminal(dpp::loglevel::ll_critical, std::format("A critical error occurred, the bot was shutdown! Trying to restart the bot in {} seconds...", s_delay));

        //artificial delay before bot restart
        std::this_thread::sleep_for(std::chrono::seconds{ s_delay });
    }

    return 0;
}

bool run_app() {
    bool is_error = true;

    mln::bot_delta::initialize_environment();
    {
        mln::bot_delta delta{};

        delta.set_request_timeout(120);

        delta.log(dpp::loglevel::ll_debug, delta.start());

        delta.log(dpp::loglevel::ll_debug, "Press any key to stop the bot...");

        while (true) {
            int c = getchar();
            if (c == 'q') {
                is_error = false;
                break;
            }
            if (c == 'p') {
                delta.print_main_db();
            } else if (c == 'd') {
                delta.log(dpp::loglevel::ll_debug, mln::database_handler::get_db_debug_info());
            }
            else if (c == 'c') {
                delta.log(dpp::loglevel::ll_debug, std::format("Total cache requests: [{}], total cache misses: [{}], cache miss rate: [{} %].", 
                    mln::caches::get_total_cache_requests(), mln::caches::get_total_cache_misses(), mln::caches::get_cache_misses_rate() * 100.0));
            }
            else if (c == 't') {
                delta.print_thread_data();
            }
        }      

        delta.log(dpp::loglevel::ll_debug, "Closing the bot...");

        while (!delta.close()) {
            
        }
    }
    mln::bot_delta::shutdown_environment();

    return is_error;
}