#pragma once
#ifndef H_MLN_DB_MOG_TEAM_DATA_H
#define H_MLN_DB_MOG_TEAM_DATA_H

#include <cstdint>
#include <string>
#include <vector>

namespace mln {
	namespace mog{
		struct team_data_t {
			struct user_data_t {
				uint64_t id;
				uint64_t cd;
				uint64_t last_update;

				user_data_t() noexcept;
				user_data_t(const uint64_t id, const uint64_t cd, const uint64_t last_update) noexcept;
				user_data_t(const user_data_t& rhs) noexcept;
				user_data_t(user_data_t&& rhs) noexcept;
				user_data_t& operator=(const user_data_t& rhs) noexcept;
				user_data_t& operator=(user_data_t&& rhs) noexcept;
			};
			std::string name;
			uint64_t guild_id;
			uint64_t channel_id;
			uint64_t role_id;
			std::vector<user_data_t> users_id_cd;

			team_data_t() noexcept;
			team_data_t(std::string name, const uint64_t guild_id, const uint64_t channel_id, const uint64_t role_id) noexcept;
			team_data_t(const team_data_t& rhs) noexcept;
			team_data_t(team_data_t&& rhs) noexcept;
			team_data_t& operator=(const team_data_t& rhs) noexcept;
			team_data_t& operator=(team_data_t&& rhs) noexcept;

			static std::string to_string(const team_data_t& data);
			static std::string to_string_partial(const team_data_t& data);
			static std::string to_string_no_cd(const team_data_t& data);
		};
	}
}

#endif // H_MLN_DB_MOG_TEAM_DATA_H