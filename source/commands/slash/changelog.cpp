#include "commands/slash/base_slashcommand.h"
#include "commands/slash/changelog.h"
#include "utility/event_data_lite.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/job.h>
#include <dpp/dispatcher.h>
#include <dpp/permissions.h>

#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

static const std::vector<std::string> s_text = {
std::format("[05/09/2024 (dd/mm/yyyy), version: {}]\nFirst bot version released.\n\n[18/09/2024 (dd/mm/yyyy), version: {}]\nAdded nsfw tag for records. Improved command responses.\n\n[21/09/2024 (dd/mm/yyyy), version: {}]\nNow the bot will check if a message to delete is too old, if that's the case it will attempt to properly delete it.\n\n[01/10/2024 (dd/mm/yyyy), version: {}]\nAdded 9 optional file parameters for the `/db insert file` command on top of the original required file parameter. This change will allow the insertion of up to 10 attachments to the database.\nAdded ability for admins to update records owned by other people in their server.\nAdded sender name when receiving a pm from `/pm` command\n\n[08/10/2024 (dd/mm/yyyy), version: {}]\nImplemented first commands for the game Eminence in Shadow: Master of Garden.\n\n[13/10/2024 (dd/mm/yyyy), version: {}]\nFixed several bugs, changed the `/mog team leave` and `/mog team leave_and_join` commands to not require the team name parameter if the user is part of a single team.\n\n[26/10/2024 (dd/mm/yyyy), version: {}]\nAdded the `/db update name` command, used to change the name of a given record.\nChanged command name from `/mog arma cooldown` to `/mog arma raw_cooldown`, now `/mog arma cooldown` is a new command that sets the cooldown automatically at 5 minutes.\n\n", 
    "0.14.0.0", "0.15.0.0", "0.15.1.0", "0.16.0.0", "0.17.0.0", "0.18.0.0", "0.18.3.0"),
};

static const uint64_t s_timeout{120};
static const uint64_t s_max_string_size{ 3000 };

mln::changelog::changelog(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand(mln::utility::prefix_dev("changelog"), "Display information about this bot's changes.", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)) } {
    cbot().log(dpp::loglevel::ll_debug, std::format("changelog: [{}].", true));
}

dpp::job mln::changelog::command(dpp::slashcommand_t event_data) {
    mln::event_data_lite_t lite_data{ event_data, bot(), true };
    if (!mln::response::is_event_data_valid(lite_data)) {
        mln::utility::create_event_log_error(lite_data, "Failed changelog, the event is incorrect!");
        co_return;
    }

    co_await mln::response::co_think(lite_data, true, false, {});

    mln::utility::manage_paginated_embed(
        mln::utility::paginated_data_t{lite_data , s_timeout, s_max_string_size }, 
        std::make_shared<const std::vector<std::string>>(s_text));
    
    co_return;
}

std::optional<std::function<void()>> mln::changelog::job(dpp::slashcommand_t)
{
    log_incorrect_command();
    return std::nullopt;
}

bool mln::changelog::use_job() const noexcept
{
    return false;
}
