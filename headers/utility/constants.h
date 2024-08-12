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

		static size_t get_min_characters_emoji();
		static size_t get_max_characters_emoji();

		static size_t get_min_retrievable_msgs();
		static size_t get_max_retrievable_msgs();
	};
}

#endif //H_MLN_DB_CONSTANTS_H