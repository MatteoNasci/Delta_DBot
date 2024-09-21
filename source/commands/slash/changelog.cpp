#include "commands/slash/changelog.h"
#include "utility/utility.h"
#include "version.h"

#include <dpp/cluster.h>

static const std::vector<std::string> s_text = {
std::format("[05/09/2024 (dd/mm/yyyy), version: {}]\nFirst bot version released.\n\n[18/09/2024 (dd/mm/yyyy), version: {}]\nAdded nsfw tag for records. Improved command responses.\n\n[21/09/2024 (dd/mm/yyyy), version: {}]\nNow the bot will check if a message to delete is too old, if that's the case it will attempt to properly delete it.\n\n", 
    "0.14.0.0", "0.15.0.0", "0.15.1.0"),
};

static const uint64_t s_timeout{120};
static const uint64_t s_max_string_size{ 3000 };

mln::changelog::changelog(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand("changelog", "Display information about this bot's changes.", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)) } {}

dpp::task<void> mln::changelog::command(const dpp::slashcommand_t& event_data) const {
    mln::utility::conf_callback_is_error(co_await event_data.co_thinking(true), bot());

    mln::utility::manage_paginated_embed(mln::utility::paginated_data_t{
        event_data.command.token, bot(),
        event_data.command.guild_id, event_data.command.channel_id, event_data.command.id,
        s_timeout, s_max_string_size }, std::make_shared<const std::vector<std::string>>(s_text));
    
    co_return;
}