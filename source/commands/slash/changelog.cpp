#include "commands/slash/changelog.h"
#include "bot_delta.h"
#include "utility/utility.h"
#include "version.h"

static const uint64_t s_timeout{120};
static const uint64_t s_max_string_size{ 3000 };

mln::changelog::changelog(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("changelog", "Display information about this bot's changes.", delta->bot.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands))), text{ std::make_shared<const std::vector<std::string>>(std::vector<std::string>{
            R"""(05/09/2024 (dd/mm/yyyy), version: )""" + std::string(mln::get_version()) + R"""(

First bot version released.)""",
        R"""()"""} ) } {}

dpp::task<void> mln::changelog::command(const dpp::slashcommand_t& event_data) {
    co_await event_data.co_thinking(true);

    mln::utility::manage_paginated_embed(mln::utility::paginated_data_t{
        event_data.command.token, &(delta()->bot),
        event_data.command.guild_id, event_data.command.channel_id, event_data.command.id,
        s_timeout, s_max_string_size }, text);
        

    co_return;
}