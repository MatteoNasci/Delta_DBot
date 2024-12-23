#include "commands/base_action.h"
#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_cmd_data.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_init_type_flag.h"
#include "commands/slash/db/db_privacy.h"
#include "database/db_saved_stmt_state.h"
#include "utility/response.h"

#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>

#include <format>

static const dpp::message s_info = dpp::message{ "**Privacy Policy**" }
.set_flags(dpp::m_ephemeral)
.add_embed(dpp::embed{}.set_description(R"""(**Effective Date:** 05/09/2024 (dd/mm/yyyy)

**1. Introduction**

Welcome to Delta! This Privacy Policy explains how we collect, use, and manage the data you provide when using our bot on Discord.

**2. Data Collection**

Our bot collects the following types of information:

- **User ID**: Your unique Discord user identifier.
- **Guild ID**: The unique identifier of the server (guild) where the bot is used.
- **Channel ID**: The unique identifier of the channel used by the bot to store data or associated to a MoG team.
- **Role ID**: The unique identifier of the role associated to a MoG team.
- **Message Content URL**: A link to a copy of the content of messages you send to Delta's database.
- **Name**: The name of the database records you create with the bot or associated to a MoG team.
- **Description**: The description you provide for database records.
- **Issue Text**: The text you provide when issuing a `/report_issue` command.

**3. Data Storage and Access**

- **Storage**: The collected data is stored in a database on a dedicated server not related to Discord. This data is not shared with any third parties outside of Discord or the server used to run the bot.
- **Access**: This information is accessible to anyone with the appropriate permissions in the specific server where the data was created. Please do not provide sensitive data to the bot, as any data supplied is accessible to everyone on that server.

**4. Data Deletion**

You have the right to request the deletion of your information. If you wish to remove your data from our records, please follow these steps:

- **For Discord Server Admins: To remove all records related to your server:**
  1. Ensure no one in your guild can use the bot's commands.
  2. Use the command `/db delete guild` in a channel of the guild you wish to remove from the database.
  3. Use the command `/db config update_dump_channel` without specifying the channel parameter.
  4. Remove all existing MoG teams using the command `/mog team delete` (to see all existing teams use `/mog team show`)
  5. Remove the bot from your guild.

- **For Users: To remove all your records from a specific Discord server:**
  1. Use the command `/db delete user` in a channel of the guild from which you wish to remove your records. Ensure you specify yourself as the user parameter.
  2. Use the command `/mog team leave` for all the teams you are in.

- **For Users: To remove all your records from all servers:**
  1. Use the command `/db delete self` in any channel of any guild where the bot has access.
  2. Use the command `/mog team leave` for all the teams in any guild you are in.

Please note that even after using these commands, some data may be retained for a period of time, such as:

- **Guild ID**: The unique identifier of the server (guild) where the bot is used. After following the steps for the admin to remove the records related to the server, this information might still remain saved in the database temporarily. A future update will include a command to remove this information as well.
- **Issue Text**: Text sent to the bot via the `/report_issue` command will be stored for some time.
- **Message Content**: When creating a record, the bot saves a copy of the Discord message content in a dedicated channel and stores a link to that message in the database. The bot will attempt to delete these stored messages when the associated record is deleted. However, this may not always succeed due to the bot no longer having permission to modify the stored message or channel, or for other reasons beyond the bot's control. Users will be notified if one or more messages could not be deleted.
)""").add_field("", R"""(

**5. Data Security**

We take reasonable measures to protect your data, but please be aware that information shared within Discord channels may be visible to users with access to those channels. We cannot guarantee the security of data transmitted over Discord.

**6. Changes to This Privacy Policy**

We may update this Privacy Policy from time to time. We encourage you to review this policy periodically to stay informed about how we are protecting your information.

**7. Contact Us**

If you have any questions or concerns about this Privacy Policy or our data practices, please contact us via direct message on Discord.

**8. Consent**

By using our bot, you consent to the collection, use, and storage of your data as described in this Privacy Policy.)"""));

mln::db_privacy::db_privacy(dpp::cluster& cluster) : base_db_command{ cluster } {
    cbot().log(dpp::loglevel::ll_debug, std::format("db_privacy: [{}].", mln::get_saved_stmt_state_text(is_db_initialized())));
}

dpp::task<void> mln::db_privacy::command(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const db_command_type)
{
    co_await mln::response::co_respond(cmd_data.data, s_info, false, "Failed to reply with the db privacy text!");
    co_return;
}

mln::db_init_type_flag mln::db_privacy::get_requested_initialization_type(const db_command_type cmd) const noexcept {
	return db_init_type_flag::none;
}

mln::db_saved_stmt_state mln::db_privacy::is_db_initialized() const noexcept
{
    return db_saved_stmt_state::initialized;
}
