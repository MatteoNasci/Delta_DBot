#pragma once
#ifndef H_MLN_DB_CONSTANTS_H
#define H_MLN_DB_CONSTANTS_H

namespace mln {
	class constants {
	public:
		[[nodiscard]] inline static constexpr size_t get_min_arma_year() noexcept
		{
			return 2024;
		}
		[[nodiscard]] inline static constexpr size_t get_max_arma_year() noexcept
		{
			return 2050;
		}

		[[nodiscard]] inline static constexpr size_t get_min_characters_reply_msg() noexcept 
		{
			return 1;
		}
		[[nodiscard]] inline static constexpr size_t get_min_nickname_length() noexcept
		{
			return 1;
		}
		[[nodiscard]] inline static constexpr size_t get_max_team_name_length() noexcept
		{
			return 32;
		}
		[[nodiscard]] inline static constexpr size_t get_min_team_name_length() noexcept
		{
			return 1;
		}
		[[nodiscard]] inline static constexpr size_t get_max_arma_cd_minutes() noexcept
		{
			return 5;
		}
		[[nodiscard]] inline static constexpr size_t get_min_arma_cd_minutes() noexcept
		{
			return 0;
		}
		[[nodiscard]] inline static constexpr size_t get_max_arma_cd_seconds() noexcept
		{
			return 59;
		}
		[[nodiscard]] inline static constexpr size_t get_min_arma_cd_seconds() noexcept
		{
			return 0;
		}
		[[nodiscard]] inline static constexpr size_t get_max_nickname_length() noexcept
		{
			return 32;
		}
		[[nodiscard]] inline static constexpr size_t get_max_characters_reply_msg() noexcept
		{
			return 2000;
		}
		[[nodiscard]] inline static constexpr size_t get_max_characters_command_option_description() noexcept
		{
			return 100;
		}

		[[nodiscard]] static size_t get_min_characters_text_id() noexcept;

		[[nodiscard]] static size_t get_max_characters_text_id() noexcept;

		[[nodiscard]] inline static constexpr size_t get_min_characters_url() noexcept
		{
			return 30;
		}

		[[nodiscard]] inline static constexpr size_t get_max_characters_url() noexcept
		{
			return 400;
		}

		[[nodiscard]] inline static constexpr size_t get_min_characters_description() noexcept
		{
			return 1;
		}

		[[nodiscard]] inline static constexpr size_t get_max_characters_description() noexcept
		{
			return 100;
		}

		[[nodiscard]] inline static constexpr size_t get_min_characters_emoji() noexcept
		{
			return 1;
		}

		[[nodiscard]] inline static constexpr size_t get_max_characters_emoji() noexcept
		{
			return 30;
		}

		[[nodiscard]] inline static constexpr size_t get_min_retrievable_msgs() noexcept
		{
			return 1;
		}

		[[nodiscard]] inline static constexpr size_t get_max_retrievable_msgs() noexcept
		{
			return 300;
		}

		[[nodiscard]] inline static constexpr size_t get_max_characters_modal_component() noexcept
		{
			return 4000;
		}

		[[nodiscard]] inline static constexpr size_t get_max_characters_embed_total() noexcept
		{
			return 6000;
		}
		[[nodiscard]] inline static constexpr size_t get_min_characters_embed() noexcept
		{
			return 1;
		}
		[[nodiscard]] inline static constexpr size_t get_max_characters_embed_title() noexcept
		{
			return 256;
		}
		[[nodiscard]] inline static constexpr size_t get_max_characters_embed_description() noexcept
		{
			return 4096;
		}
		[[nodiscard]] inline static constexpr size_t get_max_embed_fields() noexcept
		{
			return 25;
		}
		[[nodiscard]] inline static constexpr size_t get_max_characters_embed_field_name() noexcept
		{
			return 256;
		}
		[[nodiscard]] inline static constexpr size_t get_max_characters_embed_field_value() noexcept
		{
			return 1024;
		}
		[[nodiscard]] inline static constexpr size_t get_max_characters_embed_footer() noexcept
		{
			return 2048;
		}
		[[nodiscard]] inline static constexpr size_t get_max_characters_embed_author() noexcept
		{
			return 256;
		}
		[[nodiscard]] inline static constexpr size_t get_max_embeds_in_msg() noexcept
		{
			return 10;
		}

		[[nodiscard]] inline static constexpr size_t get_min_msg_bulk_delete() noexcept
		{
			return 2;
		}

		[[nodiscard]] inline static constexpr size_t get_max_msg_bulk_delete() noexcept
		{
			return 100;
		}

		[[nodiscard]] inline static constexpr long long int get_big_files_request_timeout() noexcept
		{
			return long long int{ 60 * 25 };
		}
	};
}

#endif //H_MLN_DB_CONSTANTS_H