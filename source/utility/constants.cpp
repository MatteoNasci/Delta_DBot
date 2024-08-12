#include "utility/constants.h"

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
	return 1;
}

size_t mln::constants::get_max_characters_text_id(){
	return 100;
}

size_t mln::constants::get_min_characters_emoji(){
	return 1;
}

size_t mln::constants::get_max_characters_emoji(){
	return 20;
}

size_t mln::constants::get_min_retrievable_msgs(){
	return 1;
}

size_t mln::constants::get_max_retrievable_msgs(){
	return 1000;
}
