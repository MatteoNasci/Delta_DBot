#include "utility/constants.h"
#include "bot_delta.h"

size_t mln::constants::get_min_characters_reply_msg() {
	return 1;
}
size_t mln::constants::get_max_characters_reply_msg(){
	return 2000;
}
size_t mln::constants::get_max_characters_command_option_description() {
	return 100;
}

size_t mln::constants::get_min_characters_text_id(){
	return mln::bot_delta::min_text_id_size();
}

size_t mln::constants::get_max_characters_text_id(){
	return mln::bot_delta::max_text_id_size();
}

size_t mln::constants::get_min_characters_url() {
	return 30;
}

size_t mln::constants::get_max_characters_url() {
	return 400;
}

size_t mln::constants::get_min_characters_description() {
	return 1;
}

size_t mln::constants::get_max_characters_description() {
	return 100;
}

size_t mln::constants::get_min_characters_emoji(){
	return 1;
}

size_t mln::constants::get_max_characters_emoji(){
	return 30;
}

size_t mln::constants::get_min_retrievable_msgs(){
	return 1;
}

size_t mln::constants::get_max_retrievable_msgs(){
	return 300;
}

size_t mln::constants::get_max_characters_modal_component() {
	return 4000;
}

size_t mln::constants::get_max_characters_embed_total() {
	return 6000;
}
size_t mln::constants::get_min_characters_embed() {
	return 1;
}
size_t mln::constants::get_max_characters_embed_title() {
	return 256;
}
size_t mln::constants::get_max_characters_embed_description() {
	return 4096;
}
size_t mln::constants::get_max_embed_fields() {
	return 25;
}
size_t mln::constants::get_max_characters_embed_field_name() {
	return 256;
}
size_t mln::constants::get_max_characters_embed_field_value() {
	return 1024;
}
size_t mln::constants::get_max_characters_embed_footer() {
	return 2048;
}
size_t mln::constants::get_max_characters_embed_author() {
	return 256;
}