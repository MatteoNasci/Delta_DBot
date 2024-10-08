#include "bot_delta.h"
#include "time/time.h"
#include "utility/constants.h"

size_t mln::constants::get_min_characters_reply_msg() {
	return 1;
}
size_t mln::constants::get_min_nickname_length()
{
	return 1;
}
size_t mln::constants::get_max_team_name_length()
{
	return 32;
}
size_t mln::constants::get_min_team_name_length()
{
	return 1;
}
size_t mln::constants::get_max_arma_date_length()
{
	return mln::time::get_date_to_seconds_size();
}
size_t mln::constants::get_min_arma_date_length()
{
	return mln::time::get_date_to_seconds_size();
}
size_t mln::constants::get_max_arma_cd_length()
{
	return mln::time::get_cd_to_seconds_size();
}
size_t mln::constants::get_min_arma_cd_length()
{
	return mln::time::get_cd_to_seconds_size();
}
size_t mln::constants::get_max_arma_cd_minutes()
{
	return 5;
}
size_t mln::constants::get_min_arma_cd_minutes()
{
	return 0;
}
size_t mln::constants::get_max_arma_cd_seconds()
{
	return 59;
}
size_t mln::constants::get_min_arma_cd_seconds()
{
	return 0;
}
size_t mln::constants::get_max_nickname_length()
{
	return 32;
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
size_t mln::constants::get_max_embeds_in_msg() {
	return 10;
}

size_t mln::constants::get_min_msg_bulk_delete(){
	return 2;
}

size_t mln::constants::get_max_msg_bulk_delete(){
	return 100;
}

long long int mln::constants::get_big_files_request_timeout(){
	return long long int{ 60 * 25 };
}
