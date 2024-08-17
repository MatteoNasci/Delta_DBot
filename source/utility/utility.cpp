#include "utility/utility.h"
#include "utility/constants.h"

#include <dpp/cluster.h>
#include <dpp/colors.h>

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

void mln::utility::print(const dpp::cluster& bot, const dpp::message& msg){
    bot.log(dpp::loglevel::ll_debug, "Printing msg...");

    bot.log(dpp::loglevel::ll_debug, "User: " + msg.author.get_mention());
    bot.log(dpp::loglevel::ll_debug, "Channel: " + std::to_string(msg.channel_id));
    bot.log(dpp::loglevel::ll_debug, "Content: " + msg.content);
    bot.log(dpp::loglevel::ll_debug, "Last edit: " + std::to_string(msg.edited));
    bot.log(dpp::loglevel::ll_debug, "Guild: " + std::to_string(msg.guild_id));
    bot.log(dpp::loglevel::ll_debug, "Id: " + std::to_string(msg.id));
    bot.log(dpp::loglevel::ll_debug, "Sent: " + std::to_string(msg.sent));
    bot.log(dpp::loglevel::ll_debug, "Guild: " + std::to_string(msg.webhook_id));

    for (const dpp::attachment& att : msg.attachments) {
        mln::utility::print(bot, att);
    }

    for (const dpp::component& com : msg.components) {
        mln::utility::print(bot, com);
    }

    for (const dpp::message_file_data& f : msg.file_data) {
        mln::utility::print(bot, f);
    }

    mln::utility::print(bot, msg.interaction);

    bot.log(dpp::loglevel::ll_debug, "Print msg over.");
}
void mln::utility::print(const dpp::cluster& bot, const dpp::component& com) {
    bot.log(dpp::loglevel::ll_debug, "Printing component...");

    bot.log(dpp::loglevel::ll_debug, "Custom id: " + com.custom_id);
    bot.log(dpp::loglevel::ll_debug, "Disabled: " + std::to_string(com.disabled));
    bot.log(dpp::loglevel::ll_debug, "Label: " + com.label);
    bot.log(dpp::loglevel::ll_debug, "Placeholder: " + com.placeholder);
    bot.log(dpp::loglevel::ll_debug, "Type: " + std::to_string(com.type));
    bot.log(dpp::loglevel::ll_debug, "Value: " + std::holds_alternative<std::monostate>(com.value) ? "Monostate" : (std::holds_alternative<std::string>(com.value) ? 
        std::get<std::string>(com.value) : (std::holds_alternative<std::int64_t>(com.value) ? 
            std::to_string(std::get<int64_t>(com.value)) : std::to_string(std::get<double>(com.value)))));
    bot.log(dpp::loglevel::ll_debug, "Url: " + com.url);

    bot.log(dpp::loglevel::ll_debug, "Print component over.");
}
void mln::utility::print(const dpp::cluster& bot, const dpp::attachment& att) {
    bot.log(dpp::loglevel::ll_debug, "Printing attachment...");

    bot.log(dpp::loglevel::ll_debug, "Name: " + att.filename);
    bot.log(dpp::loglevel::ll_debug, "Url: " + att.url);
    bot.log(dpp::loglevel::ll_debug, "Proxy url: " + att.proxy_url);
    bot.log(dpp::loglevel::ll_debug, "Type: " + att.content_type);
    bot.log(dpp::loglevel::ll_debug, "Desc: " + att.description);
    bot.log(dpp::loglevel::ll_debug, "Ephemeral: " + std::to_string(att.ephemeral));
    bot.log(dpp::loglevel::ll_debug, "Flags: " + std::to_string(att.flags));
    bot.log(dpp::loglevel::ll_debug, "Expire time: " + std::to_string(att.get_expire_time()));
    bot.log(dpp::loglevel::ll_debug, "Issued time: " + std::to_string(att.get_issued_time()));
    bot.log(dpp::loglevel::ll_debug, "Id: " + std::to_string(att.id));
    bot.log(dpp::loglevel::ll_debug, "Null owner: " + std::to_string((att.owner != nullptr)));
    bot.log(dpp::loglevel::ll_debug, "hws: " + std::to_string(att.height) + ", " + std::to_string(att.width) + ", " + std::to_string(att.size));

    bot.log(dpp::loglevel::ll_debug, "Print attachment over.");
}
void mln::utility::print(const dpp::cluster& bot, const dpp::message_file_data& msg_fd) {
    bot.log(dpp::loglevel::ll_debug, "Printing message_file_data...");

    bot.log(dpp::loglevel::ll_debug, "Name: " + msg_fd.name);
    bot.log(dpp::loglevel::ll_debug, "Type: " + msg_fd.mimetype);
    //bot.log(dpp::loglevel::ll_debug, "Body: " + msg_fd.content);

    bot.log(dpp::loglevel::ll_debug, "Print message_file_data over.");
}
void mln::utility::print(const dpp::cluster& bot, const dpp::message::message_interaction_struct& msg_i) {
    bot.log(dpp::loglevel::ll_debug, "Printing msg interaction...");

    bot.log(dpp::loglevel::ll_debug, "Id: " + std::to_string(msg_i.id));
    bot.log(dpp::loglevel::ll_debug, "Name: " + msg_i.name);
    bot.log(dpp::loglevel::ll_debug, "Type: " + std::to_string(msg_i.type));
    bot.log(dpp::loglevel::ll_debug, "User id: " + std::to_string(msg_i.usr.id));

    bot.log(dpp::loglevel::ll_debug, "Print msg interaction over.");
}

std::string mln::utility::get_string(mln::url_type type) {
    static const std::unordered_map<mln::url_type, std::string> s_mapped_types{
        {mln::url_type::none, "none"},
        {mln::url_type::file, "file"},
        {mln::url_type::msg, "text"},
    };
    return s_mapped_types.at(type);
}

dpp::task<std::optional<dpp::guild_member>> mln::utility::resolve_guild_member(const dpp::interaction_create_t& event_data, const dpp::snowflake& user_id){
    // If we have the guild member in the command's resolved data, return it
    const std::map<dpp::snowflake, dpp::guild_member>& member_map = event_data.command.resolved.members;
    if (const auto& member_it = member_map.find(user_id); member_it != member_map.end()) {
        co_return member_it->second;
    }

    // Try looking in guild cache
    dpp::guild* guild = dpp::find_guild(event_data.command.guild_id);
    if (guild != nullptr) {
        // Look in guild's member cache
        if (const auto& member_it = guild->members.find(user_id); member_it != guild->members.end()) {
            co_return member_it->second;
        }
    }

    // Finally if everything else failed, request API
    const dpp::confirmation_callback_t confirmation = co_await event_data.from->creator->co_guild_get_member(event_data.command.guild_id, user_id);
    if (confirmation.is_error()) {
        co_return std::nullopt; // Member not found, return empty
    }
    co_return confirmation.get<dpp::guild_member>();
}

dpp::task<bool> mln::utility::send_msg_recursively_embed(dpp::cluster& bot, const dpp::interaction_create_t& event_data, const std::function<std::string(size_t index, size_t requested_size, size_t max_size)>& get_text_callback, const dpp::snowflake& target_user, bool use_first_reply, bool broadcast) {
    static const std::function<dpp::embed(size_t)> s_get_new_embed = [](size_t fields_to_reserve) {
        dpp::embed e{};
        e   .set_color(dpp::colors::sti_blue)
            .set_title("")
            .set_author("", "", "")
            .set_description("")
            .set_thumbnail("")
            .set_image("")
            .set_footer(
                dpp::embed_footer()
                .set_text("")
                .set_icon("")
            )
            .set_timestamp(time(0));
        e.fields.reserve(fields_to_reserve);
        return std::move(e);
    };
    static const char s_next_page_text[] = "\n\nContinues next page...";
    static const char s_next_page_dm_text[] = "\n\nContinues next page, sent to you by dm...";
    //NOTE: I am putting as much data inside each field without splitting the input strings, but event with the worst input strings cases (where the most available space is wasted in the fields) I will not reach the mln::constants::get_max_embed_fields() limit of 25 fields (currently). 
    //Worst case: first input = 1 length, second input = mln::constants::get_max_characters_embed_field_value() length, wasting mln::constants::get_max_characters_embed_field_value() - 1 space with 2 fields used.
    //With this worst case scenario I can display mln::constants::get_max_characters_embed_field_value()+1 chars per every 2 fields, which will still reach the mln::constants::get_max_characters_embed_total() limit before the field count limit.
    //This means I can safely ignore the field count limit in this function
    //All of this is considering the discord embed limits of 16/08/2024

    if (!get_text_callback) {
        co_return false;
    }

    const size_t max_space_available_total = mln::constants::get_max_characters_embed_total() - (broadcast ? sizeof s_next_page_text : sizeof s_next_page_dm_text);//because I will add 'next page' text when creating new embed
    const size_t max_space_available_field = mln::constants::get_max_characters_embed_field_value();
    const size_t max_possible_fields_count = static_cast<size_t>(std::ceil((static_cast<double>(max_space_available_total) / max_space_available_field) * 2.0)) + 1; //This considers worst case scenario where 2 fields contain a max of 'max_space_available_field+1' (rounded to 'max_space_available_field' for simplicity). The + 1 is for 'next page' stuff if necessary
    size_t current_space_used = 0;
    size_t current_field_space_used = 0;

    dpp::async<dpp::confirmation_callback_t> thinking;
    bool waiting_valid = false;

    dpp::embed embed = s_get_new_embed(max_possible_fields_count);
    dpp::embed_field field{};
    bool over = false;
    size_t current_index = 0;

    while (!over) {

        size_t space_left_field = max_space_available_field - current_field_space_used;
        size_t space_left_embed = max_space_available_total - current_space_used;
        size_t available_space = std::min(space_left_field, space_left_embed);
        const std::string to_add = get_text_callback(current_index++, available_space, max_space_available_field);
        over = to_add.empty();

        //The callback has sent us a string with an illegal length amount, return an error
        if (to_add.length() > max_space_available_field) {
            co_return false;
        }

        //update current field/embed in case input string is too big for the available space or send remaining data in case process is over
        if (to_add.length() > available_space || over) {
            //If the field has at least 1 char it means that we need to push the field into the embed and prepare a new one
            if (field.value.length() > 0) {
                embed.fields.push_back(field);
                field = dpp::embed_field{};
                current_field_space_used = 0;

                space_left_field = max_space_available_field - current_field_space_used;
                available_space = std::min(space_left_field, space_left_embed);
            }

            //If the input string length is still greater than our available space it means that the embed remaining size is limiting us; the embed can be considered full and should be sent in a message, preparing a new embed after. If the process is over send any pending data left in the field/embed.
            if (to_add.length() > available_space || (over && embed.fields.size() > 0)) {

                //add new page text if the text procesing is not over
                if (!over) {
                    dpp::embed_field nxt_pg{};
                    nxt_pg.value = broadcast ? s_next_page_text : s_next_page_dm_text;
                    embed.fields.push_back(std::move(nxt_pg));
                }

                dpp::message msg{ embed };
                if (!broadcast) {
                    msg.set_flags(dpp::m_ephemeral);
                }

                if (use_first_reply) {
                    use_first_reply = !use_first_reply;
                    event_data.edit_response(msg.set_type(dpp::message_type::mt_reply));
                }else {
                    msg.set_type(dpp::message_type::mt_default);
                    if (waiting_valid) {
                        co_await thinking;
                    }
                    thinking = broadcast ? bot.co_message_create(msg.set_channel_id(event_data.command.channel_id).set_guild_id(event_data.command.guild_id)) :
                        bot.co_direct_message_create(target_user, msg);
                    waiting_valid = true;
                }

                if (!over) {
                    embed = s_get_new_embed(max_possible_fields_count);
                }
                
                current_space_used = 0;
                current_field_space_used = 0;
            }
        }
        //updates field data and counters if not over
        if(!over){
            field.value += to_add;
            current_space_used += to_add.length();
            current_field_space_used += to_add.length();
        }
    }

    if (waiting_valid) {
        co_await thinking;
    }

    co_return true;
}
dpp::task<void> mln::utility::send_msg_recursively(dpp::cluster& bot, const dpp::interaction_create_t& event_data, const std::string& in_msg, const dpp::snowflake& target_user, bool use_first_reply, bool broadcast){
    size_t text_length = in_msg.length();
    size_t start = 0;
    dpp::async<dpp::confirmation_callback_t> thinking;
    bool waiting_valid = false;
    while (text_length > 0) {
        size_t count_to_send = std::min(mln::constants::get_max_characters_reply_msg(), text_length);

        dpp::message msg{ in_msg.substr(start, count_to_send)};
        if (!broadcast) {
            msg.set_flags(dpp::m_ephemeral);
        }

        if (use_first_reply) {
            use_first_reply = !use_first_reply;
            event_data.edit_response(msg.set_type(dpp::message_type::mt_reply));
        }else {
            msg.set_type(dpp::message_type::mt_default);
            if (waiting_valid) {
                co_await thinking;
            }
            thinking = broadcast ? bot.co_message_create(msg.set_channel_id(event_data.command.channel_id).set_guild_id(event_data.command.guild_id)) :
                bot.co_direct_message_create(target_user, msg);
            waiting_valid = true;
        }

        text_length -= count_to_send;
        start += count_to_send;
    }

    if (waiting_valid) {
        co_await thinking;
    }
}
