#include "commands/dialog.h"

void dialog::form_command(bot_delta_data_t& data, const dpp::form_submit_t& event){
    /* For this simple example, we know the first element of the first row ([0][0]) is value type string.
     * In the real world, it may not be safe to make such assumptions!
     */
    std::string v = std::get<std::string>(event.components[0].components[0].value);

    dpp::message m;
    m.set_content("You entered: " + v).set_flags(dpp::m_ephemeral);

    /* Emit a reply. Form submission is still an interaction and must generate some form of reply! */
    event.reply(m);
}
void dialog::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    /* Instantiate an interaction_modal_response object */
    dpp::interaction_modal_response modal(dialog::get_custom_id(), "Please enter stuff");
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
}
dpp::slashcommand dialog::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(dialog::get_command_name(), "Make a modal dialog box", bot.me.id);
}
std::string dialog::get_command_name(){
    return "dialog";
}
std::string dialog::get_custom_id(){
    return "my_modal";
}