#include <iostream>
#include <dpp/dpp.h>

int main(int argc, char** argv){
    dpp::cluster bot(Token);
 
    bot.on_log(dpp::utility::cout_logger());

    /* The event is fired when someone issues your commands */
    bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {
        if (event.command.get_command_name() == "ping") {
            event.reply("Pong!");
        }
        if (event.command.get_command_name() == "pong") {
            event.reply("Ping!");
        }
        /* Check which command they ran */
        if (event.command.get_command_name() == "bot_info") {
            /* Create an embed */
            dpp::embed embed = dpp::embed()
                .set_color(dpp::colors::sti_blue)
                .set_title("Delta")
                .set_author("Erk_Krea", "https://dpp.dev/", "https://dpp.dev/DPP-Logo.png")
                .set_description("Bot description")
                .set_thumbnail("https://dpp.dev/DPP-Logo.png")
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
                .set_image("https://dpp.dev/DPP-Logo.png")
                .set_footer(
                    dpp::embed_footer()
                    .set_text("Some footer text here")
                    .set_icon("https://dpp.dev/DPP-Logo.png")
                )
                .set_timestamp(time(0));
 
            /* Create a message with the content as our new embed. */
            dpp::message msg(event.command.channel_id, embed);
 
            /* Reply to the user with the message, containing our embed. */
            event.reply(msg);
        }

        /* Check which command they ran */
        if (event.command.get_command_name() == "file") {
            /* Create a message. */
            dpp::message msg(event.command.channel_id, "");
 
            /* Attach the image to the message we just created. */
            msg.add_file("image.jpg", dpp::utility::read_file("D:\\Personal\\Projects\\DiscordBot\\assets\\dd6caac42dbccc609d66ee388b603118.jpg"));
 
            /* Create an embed. */
            dpp::embed embed;
            embed.set_image("attachment://image.jpg"); /* Set the image of the embed to the attached image. */
 
            /* Add the embed to the message. */
            msg.add_embed(embed);
 
            event.reply(msg);
        }
    });
 
    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("ping", "Ping pong!", bot.me.id));
            bot.global_command_create(dpp::slashcommand("pong", "Ping pong!", bot.me.id));
            /* Create and register a command when the bot is ready */
            bot.global_command_create(dpp::slashcommand("bot_info", "Send an embed with the bot info!", bot.me.id));

            bot.global_command_create(dpp::slashcommand("file", "Send a local image along with an embed with the image!", bot.me.id));
        }
    });
 
    bot.start(dpp::st_wait);
}