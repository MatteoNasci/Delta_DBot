#include "commands/slash/db/db.h"
#include "bot_delta.h"
#include "utility/constants.h"
#include "utility/utility.h"
#include "commands/slash/db/db_delete.h"
#include "commands/slash/db/db_insert.h"
#include "commands/slash/db/db_insert_update.h"
#include "commands/slash/db/db_update.h"
#include "commands/slash/db/db_update_description.h"
#include "commands/slash/db/db_select.h"
#include "commands/slash/db/db_show_records.h"
#include "commands/slash/db/db_update_dump_channel.h"

#include <dpp/queues.h>

#include <memory>

mln::db::db(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("db", "Manage the database.", delta->bot.me.id)
        .add_option(dpp::command_option(dpp::co_sub_command_group, "setup", "Collection of commands used to enhance the database features", false)
            .add_option(dpp::command_option(dpp::co_sub_command, "update_dump_channel", "Updates the dump channel used for this bot in the current server!", false)
                .add_option(dpp::command_option(dpp::co_channel, "channel", "The channel dedicated to this bot.", false).add_channel_type(dpp::channel_type::CHANNEL_TEXT))))
        .add_option(dpp::command_option(dpp::co_sub_command_group, "file", "Perform a file operation on the db", false)
            .add_option(dpp::command_option(dpp::co_sub_command, "insert", "Inserts a new record in the db. It will fail if the given name is not unique!", false)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the record.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_string, "description", "Small description of the stored record. Default: NULL", false)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))))
            .add_option(dpp::command_option(dpp::co_sub_command, "insert_update", "Inserts or replaces a record in the db.", false)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the record.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_string, "description", "Small description of the stored record. Default: NULL", false)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))))
            .add_option(dpp::command_option(dpp::co_sub_command, "update", "Updates an existing record in the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the record.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_string, "description", "Small description of the stored record. Default: NULL", false)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))))
        .add_option(dpp::command_option(dpp::co_sub_command_group, "text", "Perform a text operation on the db", false)
            .add_option(dpp::command_option(dpp::co_sub_command, "insert", "Inserts a new record in the db. It will fail if the given name is not unique!", false)
                .add_option(dpp::command_option(dpp::co_string, "text_to_store", "Text to insert.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_embed_description()))))
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the record.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_string, "description", "Small description of the stored record. Default: NULL", false)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))))
            .add_option(dpp::command_option(dpp::co_sub_command, "insert_update", "Inserts or replaces a record in the db.", false)
                .add_option(dpp::command_option(dpp::co_string, "text_to_store", "Text to insert.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_embed_description()))))
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the record.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_string, "description", "Small description of the stored record. Default: NULL", false)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))))
            .add_option(dpp::command_option(dpp::co_sub_command, "update", "Updates an existing record in the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_string, "text_to_store", "Text to insert.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_embed_description()))))
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the record.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_string, "description", "Small description of the stored record. Default: NULL", false)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))))
        .add_option(dpp::command_option(dpp::co_sub_command_group, "op", "Perform a text or file operation on the db", false)
            .add_option(dpp::command_option(dpp::co_sub_command, "select", "Selects a record from the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the record.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_boolean, "verbose", "Tells the bot to output as much info as possible about the record. Default: false", false))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "show_records", "Shows all the db records names.", false)
                .add_option(dpp::command_option(dpp::co_boolean, "verbose", "Tells the bot to output as much info as possible about the records. Default: false", false))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "update_description", "Updates an existing record description in the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the record.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_string, "description", "Small description of the stored record. Default: NULL", false)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))))
            .add_option(dpp::command_option(dpp::co_sub_command, "delete", "Removes an existing record in the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the record.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))))) {}

dpp::task<void> mln::db::command(const dpp::slashcommand_t& event){
    static const std::unordered_map<std::string, std::shared_ptr<mln::base_db_command>> s_allowed_file_sub_commands{
        {"insert", std::make_shared<mln::db_insert>(delta())},
        {"insert_update", std::make_shared<mln::db_insert_update>(delta())},
        {"update", std::make_shared<mln::db_update>(delta())},
    };
    static const std::unordered_map<std::string, std::shared_ptr<mln::base_db_command>> s_allowed_text_sub_commands{
        {"insert", s_allowed_file_sub_commands.at("insert")},
        {"insert_update", s_allowed_file_sub_commands.at("insert_update")},
        {"update", s_allowed_file_sub_commands.at("update")},
    };
    static const std::unordered_map<std::string, std::shared_ptr<mln::base_db_command>> s_allowed_op_sub_commands{
        {"update_description", std::make_shared<mln::db_update_description>(delta())},
        {"delete", std::make_shared<mln::db_delete>(delta())},
        {"select", std::make_shared<mln::db_select>(delta())},
        {"show_records", std::make_shared<mln::db_show_records>(delta())},
    };
    static const std::unordered_map<std::string, std::shared_ptr<mln::base_db_command>> s_allowed_setup_sub_commands{
        {"update_dump_channel", std::make_shared<mln::db_update_dump_channel>(delta())},
    };
    static const std::unordered_map<std::string, const std::unordered_map<std::string, std::shared_ptr<mln::base_db_command>>*> s_allowed_primary_sub_commands{
        {"file", &s_allowed_file_sub_commands},
        {"text", &s_allowed_text_sub_commands},//TODO add checks for permissions
        {"setup", &s_allowed_setup_sub_commands},
        {"op", &s_allowed_op_sub_commands},
    };
    static const std::unordered_map<std::string, mln::url_type> s_mapped_commands_to_url_type{
        {"file", mln::url_type::file},
        {"text", mln::url_type::msg},
        {"setup", mln::url_type::none},
        {"op", mln::url_type::none},
    };

    dpp::command_interaction cmd_data = event.command.get_command_interaction();
    dpp::command_data_option primary_cmd = cmd_data.options[0];
    const auto& it = s_allowed_primary_sub_commands.find(primary_cmd.name);
    if (it == s_allowed_primary_sub_commands.end()) {
        event.reply("Couldn't find primary sub_command " + primary_cmd.name);
        co_return;
    }
    
    const std::unordered_map<std::string, std::shared_ptr<mln::base_db_command>>* const mapper = it->second;
    dpp::command_data_option sub_command = primary_cmd.options[0];
    const auto& sub_it = mapper->find(sub_command.name);
    if (sub_it == mapper->end()) {
        event.reply("Couldn't find " + primary_cmd.name + "'s sub_command " + sub_command.name);
        co_return;
    }

    const auto& type_it = s_mapped_commands_to_url_type.find(primary_cmd.name);
    if (type_it == s_mapped_commands_to_url_type.end()) {
        event.reply("Couldn't find " + primary_cmd.name + "'s url type");
        co_return;
    }

    co_await sub_it->second->command(sub_command, event, type_it->second);
}