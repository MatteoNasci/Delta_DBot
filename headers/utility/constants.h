#pragma once
#ifndef H_MLN_DB_CONSTANTS_H
#define H_MLN_DB_CONSTANTS_H

namespace mln {
	class constants {
	public:
		static size_t get_max_characters_reply_msg();
		static size_t get_min_characters_reply_msg();
		static size_t get_max_characters_command_option_description();

		static size_t get_min_characters_text_id();
		static size_t get_max_characters_text_id();
		static size_t get_min_characters_description();
		static size_t get_max_characters_description();
		static size_t get_min_characters_url();
		static size_t get_max_characters_url();

		static size_t get_min_characters_emoji();
		static size_t get_max_characters_emoji();

		static size_t get_min_retrievable_msgs();
		static size_t get_max_retrievable_msgs();

		static size_t get_max_characters_modal_component();

		static size_t get_max_characters_embed_total();
		static size_t get_min_characters_embed();

		static size_t get_max_characters_embed_title();
		static size_t get_max_characters_embed_description();
		static size_t get_max_embed_fields();
		static size_t get_max_characters_embed_field_name();
		static size_t get_max_characters_embed_field_value();
		static size_t get_max_characters_embed_footer();
		static size_t get_max_characters_embed_author();
	};
}

#endif //H_MLN_DB_CONSTANTS_H