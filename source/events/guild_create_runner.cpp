#include "events/guild_create_runner.h"
#include "bot_delta.h"
#include "commands/guild/create/insert_guild_db.h"

#include <dpp/dispatcher.h>

void mln::guild_create_runner::attach_event(bot_delta* const delta) {
    actions.emplace_back(std::make_unique<mln::insert_guild_db>(delta));

    delta->bot.on_guild_create([this](const dpp::guild_create_t& event) ->dpp::job {
        std::shared_ptr<dpp::guild_create_t> ptr(std::make_shared<dpp::guild_create_t>(event));
        for (const auto& action : this->actions) {
            action->command(ptr);
        }
        co_return;
    });
}