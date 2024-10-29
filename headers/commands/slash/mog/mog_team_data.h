#pragma once
#ifndef H_MLN_DB_MOG_TEAM_DATA_H
#define H_MLN_DB_MOG_TEAM_DATA_H

#include <cstdint>
#include <string>
#include <vector>

namespace mln {
	namespace mog{
		struct mog_team_data_t {
			struct user_data_t {
				uint64_t id;
				uint64_t cd;
				uint64_t last_update;

				user_data_t() noexcept;
				user_data_t(const uint64_t id, const uint64_t cd, const uint64_t last_update) noexcept;
				user_data_t(const user_data_t& rhs) noexcept = default;
				user_data_t(user_data_t&& rhs) noexcept = default;
				user_data_t& operator=(const user_data_t& rhs) noexcept = default;
				user_data_t& operator=(user_data_t&& rhs) noexcept = default;
			};
			std::string name;
			uint64_t guild_id;
			uint64_t channel_id;
			uint64_t role_id;
			std::vector<user_data_t> users_id_cd;

			mog_team_data_t();
			mog_team_data_t(std::string name, const uint64_t guild_id, const uint64_t channel_id, const uint64_t role_id);
			mog_team_data_t(const mog_team_data_t& rhs) = default;
			mog_team_data_t(mog_team_data_t&& rhs) noexcept = default;
			mog_team_data_t& operator=(const mog_team_data_t& rhs) = default;
			mog_team_data_t& operator=(mog_team_data_t&& rhs) noexcept = default;

			static std::string to_string(const mog_team_data_t& data, const uint64_t min_valid_cd, const uint64_t min_cd_clamp);
			static std::string to_string_partial(const mog_team_data_t& data);
			static std::string to_string_no_cd(const mog_team_data_t& data);
			static bool is_member_cooldown_valid(const user_data_t& user, const uint64_t min_valid_cd) noexcept;
		};
	}
}

#endif // H_MLN_DB_MOG_TEAM_DATA_H