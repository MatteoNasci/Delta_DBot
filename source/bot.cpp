#include <iostream>

#define DPP_CORO
#include <dpp/dpp.h>
#include <dpp/unicode_emoji.h>
#include <map>
#include <functional>
#include "bot.h"

const char* deploy_delta(const bool register_cmds){
//TODO move all callbacks to their own files to tidy up this function
    dpp::cluster bot(DISCORD_BOT_TOKEN, dpp::i_default_intents | dpp::i_guild_members | dpp::i_message_content);
 
    bot.on_log(dpp::utility::cout_logger());

    const dpp::snowflake dev_id{MLN_DB_DISCORD_DEV_ID};
#ifdef MLN_DB_DISCORD_DEV_ID
    const bool is_dev_id_valid = true;
#else
    const bool is_dev_id_valid = false;
#endif
    bot.log(dpp::loglevel::ll_info, is_dev_id_valid ? "Dev id found: " : "Dev id not found!");

    const std::map<std::string, std::function<void(dpp::cluster& bot, const dpp::user_context_menu_t& event)>> cmds_ctx{
        {"High Five", [](dpp::cluster& bot, const dpp::user_context_menu_t& event) {
            dpp::user user = event.get_user(); // the user who the command has been issued on
            dpp::user author = event.command.get_issuing_user(); // the user who clicked on the context menu
            event.reply(author.get_mention() + " slapped " + user.get_mention());
        }}
    };

    const std::map<std::string, std::function<void(dpp::cluster& bot, const dpp::slashcommand_t& event)>> cmds {
        {"ping", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                event.reply("Pong!");
            }},
        {"pong", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                event.reply("Ping!");
            }},
        {"thinking", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                /*
                * true for Ephemeral. 
                * You can set this to false if you want everyone to see the thinking response.
                */
                event.thinking(true, [event](const dpp::confirmation_callback_t& callback) {
                    event.edit_original_response(dpp::message("thonk"));
                });
            }},
        {"bot_info", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                dpp::embed embed = dpp::embed()
                    .set_color(dpp::colors::sti_blue)
                    .set_title("Delta")
                    .set_author("Erk_Krea", "https://github.com/MatteoNasci/Delta_DBot", "https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
                    .set_description("Bot description")
                    .set_thumbnail("https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
                    .add_field(
                        "Regular field title",
                        "Some value here"
                    )
                    .add_field(
                        "Inline field title",
                        "Some value here",
                        true
                    )
                    .add_field(
                        "Inline field title",
                        "Some value here",
                        true
                    )
                    .set_image("https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
                    .set_footer(
                        dpp::embed_footer()
                        .set_text("Some footer text here")
                        .set_icon("https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
                    )
                    .set_timestamp(time(0));
 
                /* Create a message with the content as our new embed. */
                dpp::message msg(event.command.channel_id, embed);
 
                /* Reply to the user with the message, containing our embed. */
                event.reply(msg);
            }},
        {"file", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                dpp::message msg(event.command.channel_id, "");
 
                /* Attach the image to the message we just created. */
                msg.add_file("image.jpg", dpp::utility::read_file("D:\\Personal\\Projects\\DiscordBot\\assets\\dd6caac42dbccc609d66ee388b603118.jpg"));
    
                /* Create an embed. */
                dpp::embed embed;
                embed.set_image("attachment://image.jpg"); /* Set the image of the embed to the attached image. */
    
                /* Add the embed to the message. */
                msg.add_embed(embed);
    
                event.reply(msg);
            }},
        {"pm", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                dpp::snowflake user;
 
                /* If there was no specified user, we set the "user" variable to the command author (issuing user). */
                if (event.get_parameter("user").index() == 0) {
                    user = event.command.get_issuing_user().id;
                } else { /* Otherwise, we set it to the specified user! */
                    user = std::get<dpp::snowflake>(event.get_parameter("user"));
                }

                std::string msg;
                if (event.get_parameter("msg").index() != 0) {
                    msg = std::get<std::string>(event.get_parameter("msg"));
                }
    
                /* Send a message to the user set above. */
                bot.direct_message_create(user, dpp::message(msg.empty() ? "Ping!" : msg), [event, user](const dpp::confirmation_callback_t& callback){
                    /* If the callback errors, we want to send a message telling the author that something went wrong. */
                    if (callback.is_error()) {
                        /* Here, we want the error message to be different if the user we're trying to send a message to is the command author. */
                        if (user == event.command.get_issuing_user().id) {
                            event.reply(dpp::message("I couldn't send you a message.").set_flags(dpp::m_ephemeral));
                        } else {
                            event.reply(dpp::message("I couldn't send a message to that user. Please check that is a valid user!").set_flags(dpp::m_ephemeral));
                        }
    
                        return;
                    }
    
                    /* We do the same here, so the message is different if it's to the command author or if it's to a specified user. */
                    if (user == event.command.get_issuing_user().id) {
                        event.reply(dpp::message("I've sent you a private message.").set_flags(dpp::m_ephemeral));
                    } else {
                        event.reply(dpp::message("I've sent a message to that user.").set_flags(dpp::m_ephemeral));
                    }
                });
            }},
        {"msgs_get", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                int64_t limit = std::get<int64_t>(event.get_parameter("quantity"));
 
                /* get messages using ID of the channel the command was issued in */
                bot.messages_get(event.command.channel_id, 0, 0, 0, limit, [event](const dpp::confirmation_callback_t& callback) {
                    if (callback.is_error()) { /* catching an error to log it */
                        std::cout << callback.get_error().message << std::endl;
                        return;
                    }
    
                    auto messages = callback.get<dpp::message_map>();
                    /* std::get<dpp::message_map>(callback.value) would give the same result */
    
                    std::string contents;
                    for (const auto& x : messages) { /* here we iterate through the dpp::message_map we got from callback... */
                        contents += x.second.content + '\n'; /* ...where x.first is ID of the current message and x.second is the message itself. */
                    }
    
                    event.reply(contents); /* we will see all those messages we got, united as one! */
                });
            }},
        {"channel_create", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                /* create a text channel */
                dpp::channel channel = dpp::channel()
                    .set_name("test")
                    .set_guild_id(event.command.guild_id);
    
                bot.channel_create(channel, [&bot, event](const dpp::confirmation_callback_t& callback) -> void {
                    if (callback.is_error()) { /* catching an error to log it */
                        bot.log(dpp::loglevel::ll_error, callback.get_error().message);
                        return;
                    }
    
                    auto channel = callback.get<dpp::channel>();
                    /* std::get<dpp::channel>(callback.value) would give the same result */
    
                    /* reply with the created channel information */
                    dpp::message message = dpp::message("The channel's name is `" + channel.name + "`, ID is `" + std::to_string(channel.id) + " and type is `" + std::to_string(channel.get_type()) + "`.");
                    /* note that channel types are represented as numbers */
                    event.reply(message);
                });
            }},
        {"msg_error", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                bot.message_get(0, 0, [event](const dpp::confirmation_callback_t& callback) -> void {
                    /* the error will occur since there is no message with ID '0' that is in a channel with ID '0' (I'm not explaining why) */
                    if (callback.is_error()) {
                        event.reply(callback.get_error().message);
                        return;
                    }
    
                    /* we won't be able to get here because of the return; statement */
                    auto message = callback.get<dpp::message>();
                    event.reply(message);
                });
            }},
        {"image", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                /* Get the sub command */
                const dpp::command_interaction cmd_data = event.command.get_command_interaction();
                auto subcommand = cmd_data.options[0];
                /* Check if the subcommand is "dog" */
                if (subcommand.name == "dog") { 
                    /* Checks if the subcommand has any options. */
                    if (!subcommand.options.empty()) {
                        /* Get the user from the parameter */
                        dpp::user user = event.command.get_resolved_user(subcommand.get_value<dpp::snowflake>(0));
                        event.reply(user.get_mention() + " has now been turned into a dog."); 
                    } else {
                        /* Reply if there were no options.. */
                        event.reply("No user specified");
                    }
                } else if (subcommand.name == "cat") { /* Check if the subcommand is "cat". */
                    /* Checks if the subcommand has any options. */
                    if (!subcommand.options.empty()) {
                        /* Get the user from the parameter */
                        dpp::user user = event.command.get_resolved_user(subcommand.get_value<dpp::snowflake>(0));
                        event.reply(user.get_mention() + " has now been turned into a cat."); 
                    } else {
                        /* Reply if there were no options.. */
                        event.reply("No user specified");
                    }
                }
            }},
        {"blep", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                /* Fetch a parameter value from the command parameters */
                std::string animal = std::get<std::string>(event.get_parameter("animal"));
                /* Reply to the command. There is an overloaded version of this
                * call that accepts a dpp::message so you can send embeds.
                */
                event.reply("Blep! You chose " + animal);
            }},
        {"show", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                /* Get the file id from the parameter attachment. */
                dpp::snowflake file_id = std::get<dpp::snowflake>(event.get_parameter("file"));
    
                /* Get the attachment that the user inputted from the file id. */
                dpp::attachment att = event.command.get_resolved_attachment(file_id);
    
                /* Reply with the file as a URL. */
                event.reply(att.url);
            }},
        {"add_role", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                /* Fetch a parameter value from the command options */
                dpp::snowflake user_id = std::get<dpp::snowflake>(event.get_parameter("user"));
                dpp::snowflake role_id = std::get<dpp::snowflake>(event.get_parameter("role"));
 
                /* Get member object from resolved list */
                dpp::guild_member resolved_member = event.command.get_resolved_member(user_id);
 
                resolved_member.add_role(role_id);
                bot.guild_edit_member(resolved_member);
 
                event.reply("Added role");
            }},
        {"button", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                /* Create a message */
                dpp::message msg(event.command.channel_id, "this text has a button");

                /* Add an action row, and then a button within the action row. */
                msg.add_component(
                    dpp::component().add_component(
                        dpp::component()
                            .set_label("Click me!")
                            .set_type(dpp::cot_button)
                            .set_emoji(dpp::unicode_emoji::smile)
                            .set_style(dpp::cos_danger)
                            .set_id("myid")
                    )
                );
    
                /* Reply to the user with our message. */
                event.reply(msg);
            }},
        {"math", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                /* Create a message */
                dpp::message msg(event.command.channel_id, "What is 5+5?");

                /* Add an action row, and then 3 buttons within the action row. */
                msg.add_component(
                    dpp::component().add_component(
                        dpp::component()
                            .set_label("9")
                            .set_style(dpp::cos_primary)
                            .set_id("9")
                    )
                    .add_component(
                        dpp::component()
                            .set_label("10")
                            .set_style(dpp::cos_primary)
                            .set_id("10")
                    )
                    .add_component(
                        dpp::component()
                            .set_label("11")
                            .set_style(dpp::cos_primary)
                            .set_id("11")
                    )
                );
    
                /* Reply to the user with our message. */
                event.reply(msg);
            }},
        {"pring", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                std::string msg;
                if (event.get_parameter("testparameter").index() != 0) {
                    msg = std::get<std::string>(event.get_parameter("testparameter"));
                }
                event.reply("Prong! -> " + msg);
            }},
        {"select", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                dpp::message msg(event.command.channel_id, "This text has a select menu!");
            
                /* Add an action row, and a select menu within the action row. */
                msg.add_component(
                    dpp::component().add_component(
                        dpp::component()
                            .set_type(dpp::cot_selectmenu)
                            .set_placeholder("Pick something")
                            .add_select_option(dpp::select_option("label1","value1","description1").set_emoji(dpp::unicode_emoji::smile))
                            .add_select_option(dpp::select_option("label2","value2","description2").set_emoji(dpp::unicode_emoji::slight_smile))
                            .set_id("myselectid")
                        )
                );
 
                /* Reply to the user with our message. */
                event.reply(msg);
            }},
        {"select2", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
	            dpp::message msg(event.command.channel_id, "This text has a select menu!");
            
                /* Add an action row, and a select menu within the action row. 
                 *
                 * By default, max values is 1, meaning people can only pick 1 option.
                 * We're changing this to two, so people can select multiple options!
                 * We'll also set the min_values to 2 so people have to pick another value!
                 */
                msg.add_component(
                    dpp::component().add_component(
                        dpp::component()
                            .set_type(dpp::cot_role_selectmenu)
                            .set_min_values(2)
                            .set_max_values(2)
                            .set_id("myselect2id")
                    )
                );
    
                /* Reply to the user with our message. */
                event.reply(msg);
            }},
        {"select3", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                dpp::message msg(event.command.channel_id, "this text has a button");

                /* Add an action row, and then a button within the action row. */
                msg.add_component(
                    dpp::component().add_component(
                        dpp::component()
                            .set_label("Click me!")
                            .set_type(dpp::cot_button)
                            .set_emoji(dpp::unicode_emoji::smile)
                            .set_style(dpp::cos_danger)
                            .set_id("my2id")
                    )
                );
    
                /* Reply to the user with our message. */
                event.reply(msg);
            }},
        {"button2", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                dpp::message msg(event.command.channel_id, "This text has a select menu!");
 
                /* Add an action row, and a select menu within the action row. 
                 *
                 * Your default values are limited to max_values,
                 * meaning you can't add more default values than the allowed max values.
                 */
                msg.add_component(
                    dpp::component().add_component(
                        dpp::component()
                            .set_type(dpp::cot_role_selectmenu)
                            .set_min_values(2)
                            .set_max_values(2)
                            .add_default_value(dpp::snowflake{667756886443163648}, dpp::cdt_role)
                            .set_id("myselect3id")
                    )
                );

                /* Reply to the user with our message. */
                event.reply(msg);
            }},
        {"dialog", [](dpp::cluster& bot, const dpp::slashcommand_t& event){
                /* Instantiate an interaction_modal_response object */
                dpp::interaction_modal_response modal("my_modal", "Please enter stuff");
    
                /* Add a text component */
                modal.add_component(
                    dpp::component()
                        .set_label("Short type rammel")
                        .set_id("field_id")
                        .set_type(dpp::cot_text)
                        .set_placeholder("gumd")
                        .set_min_length(5)
                        .set_max_length(50)
                        .set_text_style(dpp::text_short)
                );
    
                /* Add another text component in the next row, as required by Discord */
                modal.add_row();
                modal.add_component(
                    dpp::component()
                        .set_label("Type rammel")
                        .set_id("field_id2")
                        .set_type(dpp::cot_text)
                        .set_placeholder("gumf")
                        .set_min_length(1)
                        .set_max_length(2000)
                        .set_text_style(dpp::text_paragraph)
                );
    
                /* Trigger the dialog box. All dialog boxes are ephemeral */
                event.dialog(modal);
            }}
    };

    /* The event is fired when someone issues your commands */
    bot.on_slashcommand([&bot, &cmds](const dpp::slashcommand_t& event) {
        const std::string cmd_name = event.command.get_command_name();

        if(cmds.contains(cmd_name)){
            cmds.at(cmd_name)(bot, event);
            bot.log(dpp::ll_info, cmd_name + " command found and executed!");
        }else{
            event.reply("Command not found!");
            bot.log(dpp::ll_error, cmd_name + " command not found!");
        }
    });

    /* Use the on_user_context_menu event to look for user context menu actions */
    bot.on_user_context_menu([&bot, &cmds_ctx](const dpp::user_context_menu_t& event) {
 
        const std::string cmd_name = event.command.get_command_name();

        if(cmds_ctx.contains(cmd_name)){
            cmds_ctx.at(cmd_name)(bot, event);
            bot.log(dpp::ll_info, cmd_name + " user context command found and executed!");
        }else{
            event.reply("Command not found!");
            bot.log(dpp::ll_error, cmd_name + " user context command not found!");
        }
    });
 
     /*Note
    Once your bot gets big, it's not recommended to create slash commands in the on_ready event even when it's inside dpp::run_once as, 
    if you re-run your bot multiple times or start multiple clusters, you will quickly get rate-limited! 
    You could, for example, add a commandline parameter to your bot (argc, argv) so that if you want the bot to register commands it must be launched with a specific command line argument.*/
    bot.on_ready([&bot, register_cmds](const dpp::ready_t& event) {
        if(!register_cmds){
            return;
        }

        if (dpp::run_once<struct register_bot_commands>()) {  
            bot.global_bulk_command_delete();
                
            constexpr int64_t msgs_get_min_val{1};
            constexpr int64_t msgs_get_max_val{100};
            bot.global_bulk_command_create({ 
                dpp::slashcommand("ping", "Ping pong!", bot.me.id),
                dpp::slashcommand("pong", "Ping pong!", bot.me.id),
                dpp::slashcommand("bot_info", "Send an embed with the bot info!", bot.me.id),
                dpp::slashcommand("file", "Send a local image along with an embed with the image!", bot.me.id),
                dpp::slashcommand("pm", "Send a private message.", bot.me.id)
                    .add_option(dpp::command_option(dpp::co_mentionable, "user", "The user to message", false))
                    .add_option(dpp::command_option(dpp::co_string, "msg", "The message to send", false)),
                dpp::slashcommand("msgs_get", "Get messages", bot.me.id)
                    .add_option(dpp::command_option(dpp::co_integer, "quantity", "Quantity of messages to get. Max - 100.")
                        .set_min_value(msgs_get_min_val).set_max_value(msgs_get_max_val)), 
                dpp::slashcommand("channel_create", "Create a channel", bot.me.id), 
                dpp::slashcommand("msg_error", "Get an error instead of message :)", bot.me.id),
                dpp::slashcommand("image", "Send a specific image.", bot.me.id)
                    .add_option(dpp::command_option(dpp::co_sub_command, "dog", "Send a picture of a dog.").add_option(dpp::command_option(dpp::co_user, "user", "User to turn into a dog.", false)))
                    .add_option(dpp::command_option(dpp::co_sub_command, "cat", "Send a picture of a cat.").add_option(dpp::command_option(dpp::co_user, "user", "User to turn into a cat.", false))),
                dpp::slashcommand("blep", "Send a random adorable animal photo", bot.me.id)
                /* If you set the auto complete setting on a command option, it will trigger the on_autocomplete
                * event whenever discord needs to fill information for the choices. You cannot set any choices
                * here if you set the auto complete value to true.
                */
                    .add_option(dpp::command_option(dpp::co_string, "animal", "The type of animal").set_auto_complete(true)),
                dpp::slashcommand("show", "Show an uploaded file", bot.me.id)
                    .add_option(dpp::command_option(dpp::co_attachment, "file", "Select an image")),
                dpp::slashcommand("add_role", "Give user a role", bot.me.id)
                /* Add user and role type command options to the slash command */
                    .add_option(dpp::command_option(dpp::co_user, "user", "User to give role to", true))
                    .add_option(dpp::command_option(dpp::co_role, "role", "Role to give", true)),
                dpp::slashcommand("button", "Send a message with a button!", bot.me.id),
                dpp::slashcommand("math", "A quick maths question!", bot.me.id),
                dpp::slashcommand("pring", "A test ping command", bot.me.id)
                    .add_option(dpp::command_option(dpp::co_string, "testparameter", "Optional test parameter")),
                dpp::slashcommand("select", "Select something at random!", bot.me.id),
                dpp::slashcommand("select2", "Select something at random!", bot.me.id),
                dpp::slashcommand("select3", "Select something at random!", bot.me.id),
                dpp::slashcommand("button2", "Send a message with a button!", bot.me.id),
                dpp::slashcommand("dialog", "Make a modal dialog box", bot.me.id),
                dpp::slashcommand("High Five", "", bot.me.id)
                    .set_type(dpp::ctxm_user),
                dpp::slashcommand("thinking", "Thinking example...", bot.me.id) });

            }
    });


    /* This event handles form submission for the modal dialog we create above */
    bot.on_form_submit([](const dpp::form_submit_t & event) {
        /* For this simple example, we know the first element of the first row ([0][0]) is value type string.
         * In the real world, it may not be safe to make such assumptions!
         */
        std::string v = std::get<std::string>(event.components[0].components[0].value);
 
        dpp::message m;
        m.set_content("You entered: " + v).set_flags(dpp::m_ephemeral);
 
        /* Emit a reply. Form submission is still an interaction and must generate some form of reply! */
        event.reply(m);
    });

    /* When a user clicks your select menu , the on_select_click event will fire,
     * containing the custom_id you defined in your select menu.
     */
    bot.on_select_click([&bot](const dpp::select_click_t & event) {
        /* Select clicks are still interactions, and must be replied to in some form to
         * prevent the "this interaction has failed" message from Discord to the user.
         */
        event.reply("You clicked " + event.custom_id + " and chose: " + event.values[0]);
    });
    /* This event is fired when someone removes their reaction from a message */
    bot.on_message_reaction_remove([&bot](const dpp::message_reaction_remove_t& event) {
        /* Find the user in the cache using his discord id */
        dpp::user* reacting_user = dpp::find_user(event.reacting_user_id);
 
        /* If user not found in cache, log and return */
        if (!reacting_user) {
            bot.log(dpp::ll_info, "User with the id " + std::to_string(event.reacting_user_id) + " was not found.");
            return;
        }
 
        bot.log(dpp::ll_info, reacting_user->format_username() + " removed his reaction.");
    });
 
    /* The event is fired when the bot detects a message in any server and any channel it has access to. */
    bot.on_message_create([&bot](const dpp::message_create_t& event) {
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
    bot.on_autocomplete([&bot](const dpp::autocomplete_t & event) {
        for (auto & opt : event.options) {
            /* The option which has focused set to true is the one the user is typing in */
            if (opt.focused) {
                /* In a real world usage of this function you should return values that loosely match
                 * opt.value, which contains what the user has typed so far. The opt.value is a variant
                 * and will contain the type identical to that of the slash command parameter.
                 * Here we can safely know it is string.
                 */
                std::string uservalue = std::get<std::string>(opt.value);
                bot.interaction_response_create(event.command.id, event.command.token, dpp::interaction_response(dpp::ir_autocomplete_reply)
                    .add_autocomplete_choice(dpp::command_option_choice("squids", std::string("lots of squids")))
                    .add_autocomplete_choice(dpp::command_option_choice("cats", std::string("a few cats")))
                    .add_autocomplete_choice(dpp::command_option_choice("dogs", std::string("bucket of dogs")))
                    .add_autocomplete_choice(dpp::command_option_choice("elephants", std::string("bottle of elephants")))
                );
                bot.log(dpp::ll_debug, "Autocomplete " + opt.name + " with value '" + uservalue + "' in field " + event.name);
                break;
            }
        }
    });


    /* When a user clicks your button, the on_button_click event will fire,
     * containing the custom_id you defined in your button.
     */
    bot.on_button_click([&bot](const dpp::button_click_t& event) {
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

    bot.start(dpp::st_wait);

    return "Success";
}
