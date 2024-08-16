#include "commands/slash/help.h"
#include "bot_delta.h"
#include "utility/utility.h"

static const size_t s_n_v_help_text_size{ 7 };
static const std::string s_n_v_help_text[s_n_v_help_text_size]{
    std::string("Delta will explain the available commands!\n\n'/db op' is the prefix for database commands, divided into Database writers and readers.\n\nDatabase writers:\n'/db op insert' file[attachment, required], name[text, required], description[text, optional] -> Inserts a new record in the database.\nIf the name is already used, an error is displayed.\nIf a dedicated bot channel exists, the file is uploaded there; otherwise, it is uploaded where the command was used.\nOnly the creator can update/delete their record.\n"),

    std::string("\n'/db op insert_update' file[attachment, required], name[text, required], description[text, optional] ->\n1) If the name isn't in the database, this acts like '/db op insert'.\n2) If the name exists, it acts like '/db op update'.\nAn error is shown if the record's owner is different.\nOnly the creator can update/delete their record.\n"),

    std::string("\n'/db op update' file[attachment, required], name[text, required], description[text, optional] -> Modifies an existing record identified by name.\nAn error is shown if the name isn't found or if the user isn't the owner.\n"),

    std::string("\n'/db op update_description' name[text, required], description[text, optional] -> Updates the description of an existing record by name.\nAn error is shown if the name isn't found or if the user isn't the owner.\n"),

    std::string("\n'/db op delete' name[text, required], broadcast[boolean, optional] -> Deletes a record from the database.\nAn error is shown if the name isn't found or if the user isn't the owner.\nIf broadcast is false, only the user sees the result (default). Otherwise, it is displayed in the channel.\n"),

    std::string("\nDatabase readers:\n'/db op show_records' verbose[boolean, optional] broadcast[boolean, optional] -> Displays all record names in the database (useful before using '/db op select').\nIf broadcast is false, only the user sees the result (default). Otherwise, it is displayed in the channel.\nIf verbose is false, only essential info is shown (default); if true, detailed info is shown.\n"),

    std::string("\n'/db op select' name[text, required] verbose[boolean, optional] broadcast[boolean, optional] -> Displays the record associated with the given name.\nRetrieves a single file by its name.\nIf broadcast is false, only the user sees the result (default). Otherwise, it is displayed in the channel.\nIf verbose is false, only essential info is shown (default); if true, detailed info is shown.\n")
};

static const size_t s_v_help_text_size{ 7 };
static const std::string s_v_help_text[s_v_help_text_size]{
    std::string("Delta will now explain the available commands!\n\n'/db op' is the prefix used by the commands related to the database. The database commands can be divided into 2 categories: Database writers and Database readers.\n\nDatabase writers:\n'/db op insert' file[attachment, required], name[text, required], description[text, optional] -> This command inserts a new record in the database.\nYou can only insert a new element. If the given name is already used by another database record, an error will be displayed.\nIf a dedicated channel has been set for the bot, the file will be uploaded there for storage purposes. Otherwise, it will be uploaded in the channel where the command was invoked.\nThe creator of a record has ownership over the record, meaning only he can update or delete it after its insertion.\n"),

    std::string("\n'/db op insert_update' file[attachment, required], name[text, required], description[text, optional]->This command behaves in 2 different ways :\n1) If a record with the given name is not present in the database, this command behaves the same as the '/db op insert' command.\n2) If a record with the given name already exists in the database, this command behaves the same as the '/db op update' command.\nIn the second case, an error will be shown if the original record was created by a different user.\nThe creator of a record has ownership over the record, meaning only he can update or delete it after its insertion.\n"),

    std::string("\n'/db op update' file[attachment, required], name[text, required], description[text, optional]->This command modifies a record already present in the database, identified by its name.\nAn error is shown if the given name is not present in the database or if the command user does not have ownership of the selected record.\n"),

    std::string("\n'/db op update_description' name[text, required], description[text, optional] -> This command modifies a record's description that is already present in the database, identified by its name.\nAn error is shown if the given name is not present in the database or if the command user does not have ownership of the selected record.\n"),

    std::string("\n'/db op delete' name[text, required], broadcast[boolean, optional] -> This command deletes a record from the database.\nAn error is shown if the given name is not present in the database or if the command user does not have ownership of the selected record.\nIf broadcast is false, only the user will see the result (default behavior if the parameter is not included in the command). Otherwise, the result will be displayed in the channel where the command was used.\n"),
    
    std::string("\nDatabase readers:\n'/db op show_records' verbose[boolean, optional] broadcast[boolean, optional] -> This command displays all the record names in the database (useful for finding the name of a record you want to select with the '/db op select' command).\nIf broadcast is false, only the user will see the result (default behavior if the parameter is not included in the command). Otherwise, the result will be displayed in the channel where the command was used.\nIf verbose is false, only the minimum required information is shown (default behavior if the parameter is not included in the command). Otherwise, the maximum amount of information is shown (most of which might not be useful to the command user).\n"),
    
    std::string("\n'/db op select' name[text, required] verbose[boolean, optional] broadcast[boolean, optional] -> This command displays the record associated with the given name if present in the database.\nThis is the main command for reading from the database, allowing you to retrieve a single file from it using its associated name.\nIf broadcast is false, only the user will see the result (default behavior if the parameter is not included in the command). Otherwise, the result will be displayed in the channel where the command was used.\nIf verbose is false, only the minimum required information is shown (default behavior if the parameter is not included in the command). Otherwise, the maximum amount of information is shown (most of which might not be useful to the command user).\n")
};
static const std::string s_empty{""};

mln::help::help(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("help", "Display information about this bot's commands.", delta->bot.me.id)
        .add_option(dpp::command_option(dpp::command_option_type::co_boolean, "verbose", "Tells the bot to output as much info as possible about itself. Default: false", false)))) {}

dpp::task<void> mln::help::command(const dpp::slashcommand_t& event_data){
    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(true);
    const dpp::command_value verbose_param = event_data.get_parameter("verbose");
    const bool verbose = std::holds_alternative<bool>(verbose_param) ? std::get<bool>(verbose_param) : false;

    size_t current_index = 0;
    const std::function<std::string(size_t, size_t)> v_func = [this, &current_index](size_t, size_t) {
        return current_index >= s_v_help_text_size ? s_empty : s_v_help_text[current_index++];
    };
    const std::function<std::string(size_t, size_t)> n_v_func = [this, &current_index](size_t, size_t) {
        return current_index >= s_n_v_help_text_size ? s_empty : s_n_v_help_text[current_index++];
    };
    
    co_await thinking;
    bool result = co_await mln::utility::send_msg_recursively_embed(delta()->bot, event_data, verbose ? v_func : n_v_func, event_data.command.usr.id, true, false);
    if (!result) {
        delta()->bot.log(dpp::loglevel::ll_error, "An error occurred while sending embeds recursively for /help command!");
    }
}