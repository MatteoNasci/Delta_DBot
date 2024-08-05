#include "bot.h"
#include "bot_delta.h"

void deploy_delta(const bool register_cmds){
    //TODO use coro in library (defined in cmake now)
    bot_delta delta(register_cmds);

    /*const std::map<std::string, on_message_reaction_remove_t> msg_reaction_remove_map{
        
    };

    const std::map<std::string, on_message_create_t> msg_create_map{
        
    };

    const std::map<std::string, on_autocomplete_t> autocomplete_map{
        
    };

    const std::map<std::string, on_button_click_t> button_click_map{
        
    };*/

    /* This event is fired when someone removes their reaction from a message */
    delta.data.bot.on_message_reaction_remove([&delta](const dpp::message_reaction_remove_t& event) {
        /* Find the user in the cache using his discord id */
        dpp::user* reacting_user = dpp::find_user(event.reacting_user_id);
 
        /* If user not found in cache, log and return */
        if (!reacting_user) {
            delta.data.bot.log(dpp::ll_info, "User with the id " + std::to_string(event.reacting_user_id) + " was not found.");
            return;
        }
 
        delta.data.bot.log(dpp::ll_info, reacting_user->format_username() + " removed his reaction.");
    });
 
    /* The event is fired when the bot detects a message in any server and any channel it has access to. */
    delta.data.bot.on_message_create([&delta](const dpp::message_create_t& event) {
        /* See if the message contains the phrase we want to check for.
         * If there's at least a single match, we reply and say it's not allowed.
         */
        if (event.msg.content.find("bad word") != std::string::npos) {
            event.reply("That is not allowed here. Please, mind your language!", true);
        }
    });

    /* The on_autocomplete event is fired whenever discord needs information to fill in a command options's choices.
     * You must reply with a REST event within 500ms, so make it snappy!
     */
    delta.data.bot.on_autocomplete([&delta](const dpp::autocomplete_t & event) {
        for (auto & opt : event.options) {
            /* The option which has focused set to true is the one the user is typing in */
            if (opt.focused) {
                /* In a real world usage of this function you should return values that loosely match
                 * opt.value, which contains what the user has typed so far. The opt.value is a variant
                 * and will contain the type identical to that of the slash command parameter.
                 * Here we can safely know it is string.
                 */
                std::string uservalue = std::get<std::string>(opt.value);
                delta.data.bot.interaction_response_create(event.command.id, event.command.token, dpp::interaction_response(dpp::ir_autocomplete_reply)
                    .add_autocomplete_choice(dpp::command_option_choice("squids", std::string("lots of squids")))
                    .add_autocomplete_choice(dpp::command_option_choice("cats", std::string("a few cats")))
                    .add_autocomplete_choice(dpp::command_option_choice("dogs", std::string("bucket of dogs")))
                    .add_autocomplete_choice(dpp::command_option_choice("elephants", std::string("bottle of elephants")))
                );
                delta.data.bot.log(dpp::ll_debug, "Autocomplete " + opt.name + " with value '" + uservalue + "' in field " + event.name);
                break;
            }
        }
    });


    /* When a user clicks your button, the on_button_click event will fire,
     * containing the custom_id you defined in your button.
     */
    delta.data.bot.on_button_click([&delta](const dpp::button_click_t& event) {
        /* Button clicks are still interactions, and must be replied to in some form to
         * prevent the "this interaction has failed" message from Discord to the user.
         */
        if (event.custom_id == "10") {
            event.reply(dpp::message("You got it right!").set_flags(dpp::m_ephemeral));
        } else if(event.custom_id == "9" || event.custom_id == "11") {
            event.reply(dpp::message("Wrong! Try again.").set_flags(dpp::m_ephemeral));
        }else if(event.custom_id == "my2id"){
            /* Instead of replying to the button click itself,
            * we want to update the message that had the buttons on it.
            */
            event.reply(dpp::ir_deferred_channel_message_with_source, "");
 
            /* Pretend you're doing long calls here that may take longer than 3 seconds. */
 
            /* Now, edit the response! */
            event.edit_response("After a while, I can confirm you clicked: " + event.custom_id);
        }else{
            event.reply("You clicked: " + event.custom_id);
        }
        
    });

    

    delta.data.bot.log(dpp::loglevel::ll_info, delta.start());
}
