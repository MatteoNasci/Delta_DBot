#include "utility/utility.h"
#include "utility/constants.h"
#include "utility/json_err.h"

#include <dpp/colors.h>
#include <dpp/channel.h>
#include <dpp/cache.h>
#include <dpp/unicode_emoji.h>
#include <dpp/cluster.h>

#include <regex>
#include <format>

bool extract_attachment_url(const std::regex& url_regex, const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
bool extract_attachment_url(const std::regex& url_regex, const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& out_name);

void mln::utility::any_results_da_callback(void* d, int, mln::db_column_data_t&&) {
    bool* const bool_ptr = static_cast<bool*>(d);
    *bool_ptr = true;
}
bool mln::utility::any_results_td_callback(void*, int) {
    return false;
}
const mln::database_callbacks_t& mln::utility::get_any_results_callback() {
    static const mln::database_callbacks_t callbacks{nullptr, mln::utility::any_results_da_callback, mln::utility::any_results_td_callback, nullptr, nullptr, nullptr};
    
    return callbacks;
}
mln::database_callbacks_t mln::utility::get_any_results_callback(bool* found) {
    mln::database_callbacks_t copy{get_any_results_callback()};
    copy.callback_data = static_cast<void*>(found);
    
    return copy;
}

bool mln::utility::extract_message_url_data(const std::string& msg_url, uint64_t& out_guild_id, uint64_t& out_channel_id, uint64_t& out_message_id) {
    static const std::regex url_regex(R"(https://discord.com/channels/(\d+)/(\d+)/(\d+))");
    std::smatch match;

    const bool regex_result = std::regex_search(msg_url, match, url_regex);
    const bool valid_results = regex_result && match.size() == 4;
    if (valid_results) {
        out_guild_id = std::stoull(match[1].str());
        out_channel_id = std::stoull(match[2].str());
        out_message_id = std::stoull(match[3].str());
    }

    return valid_results;
}
std::string mln::utility::get_current_date_time()
{
    time_t now = std::time(0);
    struct tm tstruct = *std::localtime(&now);
    char buf[80];
    std::strftime(buf, sizeof(buf), "%d-%m-%Y %X", &tstruct);
    return std::string{ buf };
}
bool mln::utility::extract_generic_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id) {
    bool result = mln::utility::extract_attachment_url_data(attachment_url, out_guild_id, out_channel_id);
    if (!result) {
        result = mln::utility::extract_ephemeral_attachment_url_data(attachment_url, out_guild_id, out_channel_id);
    }
    return result;
}
bool mln::utility::extract_generic_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name) {
    bool result = mln::utility::extract_attachment_url_data(attachment_url, out_guild_id, out_channel_id, name);
    if (!result) {
        result = mln::utility::extract_ephemeral_attachment_url_data(attachment_url, out_guild_id, out_channel_id, name);
    }
    return result;
}
bool mln::utility::extract_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id) {
    static const std::regex url_regex(R"(https://cdn.discordapp.com/attachments/(\d+)/(\d+)/)");
    return extract_attachment_url(url_regex, attachment_url, out_guild_id, out_channel_id);
}
bool mln::utility::extract_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name) {
    static const std::regex url_regex(R"(https://cdn.discordapp.com/attachments/(\d+)/(\d+)/([^\/?]+))");
    return extract_attachment_url(url_regex, attachment_url, out_guild_id, out_channel_id, name);
}
bool mln::utility::extract_ephemeral_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id) {
    static const std::regex url_regex(R"(https://cdn.discordapp.com/ephemeral-attachments/(\d+)/(\d+)/)");
    return extract_attachment_url(url_regex, attachment_url, out_guild_id, out_channel_id);
}
bool mln::utility::extract_ephemeral_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name) {
    static const std::regex url_regex(R"(https://cdn.discordapp.com/ephemeral-attachments/(\d+)/(\d+)/([^\/?]+))");
    return extract_attachment_url(url_regex, attachment_url, out_guild_id, out_channel_id, name);
}
bool extract_attachment_url(const std::regex& url_regex, const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id) {
    std::smatch match;

    const bool regex_result = std::regex_search(attachment_url, match, url_regex);
    const bool valid_results = regex_result && match.size() == 3;
    if (valid_results) {
        out_guild_id = std::stoull(match[1].str());
        out_channel_id = std::stoull(match[2].str());
    }

    return valid_results;
}
bool extract_attachment_url(const std::regex& url_regex, const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& out_name) {
    std::smatch match;

    const bool regex_result = std::regex_search(attachment_url, match, url_regex);
    const bool valid_results = regex_result && match.size() == 4;
    if (valid_results) {
        out_guild_id = std::stoull(match[1].str());
        out_channel_id = std::stoull(match[2].str());
        out_name = std::move(match[3].str());
    }

    return valid_results;
}
dpp::job mln::utility::manage_paginated_embed(paginated_data_t data, const std::shared_ptr<const std::vector<std::string>> text_ptr) {
    static const dpp::message s_msg_template = 
        dpp::message{"This message is displaying paginated information, use the buttons below to switch the displayed page!"}
        .add_component(
            dpp::component{}.set_type(dpp::component_type::cot_action_row)
            .add_component(
                dpp::component{}
                .set_label("first page")
                .set_type(dpp::component_type::cot_button)
                .set_emoji(dpp::unicode_emoji::rewind)
                .set_style(dpp::component_style::cos_primary))
            .add_component(
                dpp::component{}
                .set_label("prev page")
                .set_type(dpp::component_type::cot_button)
                .set_emoji(dpp::unicode_emoji::arrow_backward)
                .set_style(dpp::component_style::cos_primary))
            .add_component(
                dpp::component{}
                .set_label("next page")
                .set_type(dpp::component_type::cot_button)
                .set_emoji(dpp::unicode_emoji::arrow_forward)
                .set_style(dpp::component_style::cos_primary))
            .add_component(
                dpp::component{}
                .set_label("last page")
                .set_type(dpp::component_type::cot_button)
                .set_emoji(dpp::unicode_emoji::fast_forward)
                .set_style(dpp::component_style::cos_primary))
    );

    if (data.bot.me.id == 0 || data.token.empty()) {
        std::cerr << "Error while executing paginated embed job, no cluster or token found!\n";
        co_return;
    }

    if (data.channel_id == 0 || data.guild_id == 0) {
        data.bot.log(dpp::loglevel::ll_error, "Error while executing paginated embed job, no guild_id or channel_id found!");
        co_return;
    }

    if (data.time_limit_seconds == 0) {
        data.bot.log(dpp::loglevel::ll_error, "Error while executing paginated embed job, the time_limit_seconds cannot be == 0!");
        data.bot.interaction_response_edit(data.token,
            dpp::message{"Error while executing paginated embed job, the time_limit_seconds cannot be == 0!"}
            .set_guild_id(data.guild_id)
            .set_channel_id(data.channel_id));
        co_return;
    }

    if (!text_ptr || text_ptr->size() == 0) {
        data.bot.log(dpp::loglevel::ll_error, "Error while executing paginated embed job, the given text list is either invalid or empty!");
        data.bot.interaction_response_edit(data.token,
            dpp::message{"Error while executing paginated embed job, the given text list is either invalid or empty!"}
            .set_guild_id(data.guild_id)
            .set_channel_id(data.channel_id));
        co_return;
    }

    dpp::async<dpp::confirmation_callback_t> processing = data.bot.co_interaction_response_edit(data.token,
        dpp::message{"Processing the data, please wait..."}
        .set_guild_id(data.guild_id)
        .set_channel_id(data.channel_id));

    std::vector<dpp::message> msgs{};
    msgs.emplace_back(dpp::message{s_msg_template}.set_guild_id(data.guild_id).set_channel_id(data.channel_id));

    dpp::embed current_embed{};
    const std::vector<std::string>& input_strings = *text_ptr;
    size_t current_size = 0;
    size_t current_msgs_index = 0;
    const size_t max_desc_size = std::min(data.text_limit, mln::constants::get_max_characters_embed_description());
    //Fill the msgs array with all the msgs required to display all the input text
    for (const std::string& input : input_strings) {
        if (input.size() == 0) {
            data.bot.log(dpp::loglevel::ll_warning, "Found empty string while creating paginated embeds!");
            continue;
        }

        //The +3 refers to the "\n\n" added to the input.
        const size_t new_string_size = input.size() + 3;

        if (new_string_size > max_desc_size) {
            data.bot.log(dpp::loglevel::ll_warning, "Found string that exceeds the max char limits while creating paginated embeds!");
            continue;
        }

        const bool exceeded_embed_limit = (current_size + new_string_size > max_desc_size);
        //If the new string exceeds the remaining text space, submit embed to message (and, if required, submit msg to msg list) and prepare new one. 
        if (exceeded_embed_limit) {
            msgs[current_msgs_index].add_embed(current_embed);
            current_embed = dpp::embed{};

            ++current_msgs_index;
            msgs.emplace_back(dpp::message{s_msg_template}.set_guild_id(data.guild_id).set_channel_id(data.channel_id));

            current_size = 0;
        }

        //Add new string to embed and update size counter
        current_embed.description += input + "\n\n";
        current_size += new_string_size;
    }

    if (current_size > 0) {
        msgs[current_msgs_index].add_embed(current_embed);
    }

    //If only 1 msg present and embeds is empty (for example when all input strings are empty), return an error
    if (msgs.size() == 1 && msgs[current_msgs_index].embeds.size() == 0) {
        data.bot.log(dpp::loglevel::ll_error, "Error while executing paginated embed job, the given text list is filled with empty strings!");
        data.bot.interaction_response_edit(data.token,
            dpp::message{"Error while executing paginated embed job, the given text list is empty! Internal error."}
            .set_guild_id(data.guild_id)
            .set_channel_id(data.channel_id));
        co_return;
    }

    const std::string base_id{std::to_string(data.event_id)};
    const std::array<std::string, 4> button_ids{base_id + "L", base_id + "l", base_id + "r", base_id + "R"};
    const bool disable_buttons = msgs.size() <= 1;
    size_t index = 1;
    for (dpp::message& m : msgs) {
        m.content += " Page " + std::to_string(index++) + "/" + std::to_string(msgs.size());

        dpp::component& c = m.components[0];

        c.components[0].custom_id = button_ids[0];
        c.components[0].disabled = disable_buttons;

        c.components[1].custom_id = button_ids[1];
        c.components[1].disabled = disable_buttons;

        c.components[2].custom_id = button_ids[2];
        c.components[2].disabled = disable_buttons;

        c.components[3].custom_id = button_ids[3];
        c.components[3].disabled = disable_buttons;
    }

    //If only one message, skip pagination and just display the message
    if (msgs.size() == 1) {
        data.bot.interaction_response_edit(data.token, msgs[0]);
        co_return;
    }

    //Manage the pages by switching between them (if needed).
    current_msgs_index = 1;
    size_t next_msgs_index = 0;
    dpp::button_click_t button_data{};
    button_data.command.id = 0;
    while (true) {
        if (current_msgs_index != next_msgs_index) {
            current_msgs_index = next_msgs_index;

            co_await data.bot.co_interaction_response_edit(data.token, msgs[current_msgs_index]);
        }

        if (button_data.command.id != 0) {
            //Positive reply to the button press
            co_await button_data.co_reply();
        }

        const auto& result = co_await dpp::when_any{
            data.bot.co_sleep(data.time_limit_seconds),
            data.bot.on_button_click.when([&button_ids](const dpp::button_click_t& event_data) {
                for (const std::string s : button_ids) {
                    if (s == event_data.custom_id) {
                        return true;
                    }
                }
                return false;
                })};

        //If the timer run out return an error
        if (result.index() == 0) {
            data.bot.interaction_response_edit(data.token, dpp::message{"Too much time has passed since the last interaction, the command execution has terminated"});
            co_return;
        }

        //If an exception occurred return an error
        if (result.is_exception()) {
            data.bot.interaction_response_edit(data.token, dpp::message{"An unknown error occurred!"});
            co_return;
        }

        //It was suggested to copy the event from documentation of ::when
        button_data = result.get<1>();
        if (button_data.cancelled) {
            data.bot.interaction_response_edit(data.token, dpp::message{"The event was cancelled!"});
            co_return;
        }

        if (button_data.custom_id == button_ids[0]) {
            next_msgs_index = 0;
        } else if (button_data.custom_id == button_ids[1]) {
            if (next_msgs_index != 0) {
                --next_msgs_index;
            }
        } else if (button_data.custom_id == button_ids[2]) {
            if (next_msgs_index != msgs.size() - 1) {
                ++next_msgs_index;
            }
        } else if (button_data.custom_id == button_ids[3]) {
            next_msgs_index = msgs.size() - 1;
        } else {
            data.bot.interaction_response_edit(data.token, dpp::message{"Invalid button id found, internal error!"});
            co_return;
        }

    }

    co_return;
}

bool mln::utility::is_ascii(const std::string& text) {
    return mln::utility::is_ascii(text.c_str());
}
bool mln::utility::is_ascii(const char* const text) {
    return mln::utility::is_ascii(reinterpret_cast<const unsigned char* const>(text));
}
bool mln::utility::is_ascii(const unsigned char* const text) {
    static const unsigned char bit_to_check = (1 << 7);
    for (size_t i = 0;;++i) {
        const unsigned char c = text[i];
        if (c == '\0') {
            break;
        }
        if (c & bit_to_check) {
            return false;
        }
    }
    return true;
}

bool mln::utility::is_ascii_printable(const std::string& text) {
    return mln::utility::is_ascii_printable(text.c_str());
}
bool mln::utility::is_ascii_printable(const char* const text) {
    return mln::utility::is_ascii_printable(reinterpret_cast<const unsigned char* const>(text));
}
bool mln::utility::is_ascii_printable(const unsigned char* const text) {
    static const unsigned char min_limit = 32;
    static const unsigned char max_limit = 126;
    for (size_t i = 0;;++i) {
        const unsigned char c = text[i];
        if (c == '\0') {
            break;
        }
        if (c < min_limit || c > max_limit) {
            return false;
        }
    }
    return true;
}

std::vector<dpp::snowflake> mln::utility::extract_emojis(const std::string& content) {
    static const std::regex s_custom_emoji_regex("<a?:\\w+:(\\d+)>");
    std::vector<dpp::snowflake> result{};
    std::smatch matches;

    std::string to_check = content;
    while (std::regex_search(to_check, matches, s_custom_emoji_regex)) {
        result.push_back(std::stoull(matches[1].str()));

        to_check = matches.suffix().str();
    }

    return result;
}

bool mln::utility::conf_callback_is_error(const dpp::confirmation_callback_t& callback, const dpp::cluster& bot, const dpp::interaction_create_t* const event_data, const std::string& additional_msg)
{
    const bool is_error = callback.is_error();

    if (is_error) {
        const dpp::error_info err = callback.get_error();
        if (additional_msg.empty()) {
            bot.log(dpp::loglevel::ll_error, std::format("Error found when checking dpp::confirmation_callback_t! Error: [{}], details: [{}].",
                mln::get_json_err_text(err.code), err.human_readable));
        }
        else {
            bot.log(dpp::loglevel::ll_error, std::format("Error found when checking dpp::confirmation_callback_t! Error: [{}], details: [{}]. Additional msg: [{}].",
                mln::get_json_err_text(err.code), err.human_readable, additional_msg));
        }
    }

    if (event_data) {
        mln::utility::create_event_log_error(*event_data, bot, additional_msg);
    }

    return is_error;
}

void mln::utility::create_event_log_error(const dpp::interaction_create_t& event_data, const dpp::cluster& bot, const std::string& additional_msg)
{
    if (additional_msg.empty()) {
        bot.log(dpp::loglevel::ll_error, std::format("Event error! Event id: [{}], guild: [{}], channel: [{}], user: [{}], command: [{}].", 
            static_cast<uint64_t>(event_data.command.id), static_cast<uint64_t>(event_data.command.guild_id), static_cast<uint64_t>(event_data.command.channel_id),
            static_cast<uint64_t>(event_data.command.usr.id), event_data.command.get_command_name()));
        return;
    }

    bot.log(dpp::loglevel::ll_error, std::format("Event error! Event id: [{}], guild: [{}], channel: [{}], user: [{}], command: [{}], additional info: [{}].",
        static_cast<uint64_t>(event_data.command.id), static_cast<uint64_t>(event_data.command.guild_id), static_cast<uint64_t>(event_data.command.channel_id),
        static_cast<uint64_t>(event_data.command.usr.id), event_data.command.get_command_name(), additional_msg));
}

bool mln::utility::is_same_cmd(const dpp::slashcommand& first, const dpp::slashcommand& second)
{
    bool equal = first.default_member_permissions == second.default_member_permissions && 
        first.dm_permission == second.dm_permission &&
        first.nsfw == second.nsfw &&
        first.type == second.type &&
        first.options.size() == second.options.size() &&
        first.name_localizations.size() == second.name_localizations.size() &&
        first.description_localizations.size() == second.description_localizations.size() &&
        first.name == second.name &&
        first.description == second.description;
    
    if (equal) { 
        for (const std::pair<std::string, std::string>& pair : first.description_localizations) {
            const std::map<std::string, std::string>::const_iterator& it = second.description_localizations.find(pair.first);
            if (it == second.description_localizations.end() || it->second != pair.second) {
                equal = false;
                break;
            }
        }

        if (equal) {
            for (const std::pair<std::string, std::string>& pair : first.name_localizations) {
                const std::map<std::string, std::string>::const_iterator& it = second.name_localizations.find(pair.first);
                if (it == second.name_localizations.end() || it->second != pair.second) {
                    equal = false;
                    break;
                }
            }

            if (equal) {
                for (size_t i = 0; i < first.options.size(); ++i) {
                    if (!mln::utility::is_same_option(first.options[i], second.options[i])) {
                        equal = false;
                        break;
                    }
                }
            }
        }
    }

    return equal;
}

bool mln::utility::is_same_option(const dpp::command_option& first, const dpp::command_option& second)
{
    bool equal = first.autocomplete == second.autocomplete &&
        first.required == second.required &&
        first.type == second.type &&
        first.channel_types.size() == second.channel_types.size() &&
        first.choices.size() == second.choices.size() &&
        first.description_localizations.size() == second.description_localizations.size() &&
        first.name_localizations.size() == second.name_localizations.size() &&
        first.options.size() == second.options.size() &&
        first.min_value == second.min_value &&
        first.max_value == second.max_value &&
        first.name == second.name &&
        first.description == second.description;

    if (equal) {
        for (size_t i = 0; i < first.channel_types.size(); ++i) {
            if (first.channel_types[i] != second.channel_types[i]) {
                equal = false;
                break;
            }
        }

        if (equal) {
            for (const std::pair<std::string, std::string>& pair : first.description_localizations) {
                const std::map<std::string, std::string>::const_iterator& it = second.description_localizations.find(pair.first);
                if (it == second.description_localizations.end() || it->second != pair.second) {
                    equal = false;
                    break;
                }
            }

            if (equal) {
                for (const std::pair<std::string, std::string>& pair : first.name_localizations) {
                    const std::map<std::string, std::string>::const_iterator& it = second.name_localizations.find(pair.first);
                    if (it == second.name_localizations.end() || it->second != pair.second) {
                        equal = false;
                        break;
                    }
                }

                if (equal) {
                    for (size_t i = 0; i < first.choices.size(); ++i) {
                        if (!mln::utility::is_same_choice(first.choices[i], second.choices[i])) {
                            equal = false;
                            break;
                        }
                    }

                    if (equal) {
                        for (size_t i = 0; i < first.options.size(); ++i) {
                            if (!mln::utility::is_same_option(first.options[i], second.options[i])) {
                                equal = false;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    return equal;
}

bool mln::utility::is_same_choice(const dpp::command_option_choice& first, const dpp::command_option_choice& second)
{
    bool equal = first.value == second.value && 
        first.name_localizations.size() == second.name_localizations.size() && 
        first.name == second.name;

    if (equal) {
        for (const std::pair<std::string, std::string>& pair : first.name_localizations) {
            const std::map<std::string, std::string>::const_iterator& it = second.name_localizations.find(pair.first);
            if (it == second.name_localizations.end() || it->second != pair.second) {
                equal = false;
                break;
            }
        }
    }

    return equal;
}