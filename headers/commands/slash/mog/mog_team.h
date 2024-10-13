#pragma once
#ifndef H_MLN_DB_MOG_TEAM_H
#define H_MLN_DB_MOG_TEAM_H

#include "commands/slash/mog/base_mog_command.h"
#include "commands/slash/mog/mog_command_type.h"
#include "commands/slash/mog/mog_init_type_flag.h"
#include "commands/slash/mog/mog_team_data.h"
#include "database/db_saved_stmt_state.h"

#include <dpp/coro/task.h>

#include <cstdint>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace dpp {
	class cluster;
	struct slashcommand_t;
}

namespace mln {
	class database_handler;

	namespace mog {
		struct mog_cmd_data_t;

		class mog_team final : public base_mog_command {
		private:
			struct data_t {
				size_t saved_stmt;
				int saved_param_guild, saved_param_name, saved_param_channel, saved_param_role;
				db_saved_stmt_state state;
			};
			struct del_data_t {
				size_t saved_stmt;
				int saved_param_guild, saved_param_name;
				db_saved_stmt_state state;
			};
			struct member_data_t {
				size_t saved_stmt;
				int saved_param_guild, saved_param_name, saved_param_user;
				db_saved_stmt_state state;
			};
			typedef del_data_t show_team_data_t;
			struct show_data_t {
				size_t saved_stmt;
				db_saved_stmt_state state;
			};

			mutable std::shared_mutex teams_mutex;
			mutable std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>> teams_data_cache;

			data_t data;
			del_data_t del_data;
			member_data_t member_data;
			member_data_t del_member_data;
			show_data_t show_data;
			show_team_data_t show_team_data;
			show_data_t show_all_data;

			database_handler& db;
		public:
			mog_team(dpp::cluster& cluster, database_handler& db);
			~mog_team();
			mog_team(const mog_team&) = delete;
			mog_team(mog_team&& rhs) noexcept;
			mog_team& operator=(const mog_team&) = delete;
			mog_team& operator=(mog_team&& rhs) noexcept;

			dpp::task<void> command(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data, const mog_command_type type) override final;
			mog_init_type_flag get_requested_initialization_type(const mog_command_type cmd) const noexcept override final;
			db_saved_stmt_state is_db_initialized() const noexcept override final;
		private:
			dpp::task<void> create(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const;
			dpp::task<void> del(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const;
			dpp::task<void> join(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const;
			dpp::task<void> join(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data, const mln::mog::mog_team_data_t::user_data_t& user_data, const std::string& name) const;
			dpp::task<std::optional<mln::mog::mog_team_data_t::user_data_t>> leave(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const;
			dpp::task<std::optional<mln::mog::mog_team_data_t::user_data_t>> leave(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data, const uint64_t target, const std::string& name) const;
			dpp::task<void> leave_and_join(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const;
			dpp::task<void> show(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const;
			dpp::task<void> help(mog_cmd_data_t& cmd_data) const;

			void load_teams() const;

		public:
			std::optional<mln::mog::mog_team_data_t> get_team(const uint64_t guild_id, const std::string& team_name) const;
			std::optional<mln::mog::mog_team_data_t> get_team(const uint64_t guild_id, const uint64_t user_id) const;
			void get_teams(const uint64_t guild_id, std::vector<mln::mog::mog_team_data_t>& out_teams) const;

			bool is_any_team_in_guild(const uint64_t guild_id) const;
			bool is_team_present(const uint64_t guild_id, const std::string& team_name) const;
			bool is_user_in_any_team(const uint64_t guild_id, const uint64_t user_id) const;
			size_t teams_with_user(const uint64_t guild_id, const uint64_t user_id) const;
			bool is_user_in_team(const uint64_t guild_id, const uint64_t user_id, const std::string& team_name) const;
			size_t teams_count() const;
			size_t guild_teams_count(const uint64_t guild_id) const;

			static bool is_team_valid(const mln::mog::mog_team_data_t& team) noexcept;
			static bool is_team_valid(const uint64_t guild_id, const std::string& team_name) noexcept;

			bool set_user_cooldown(const uint64_t guild_id, const std::string& team_name, const mln::mog::mog_team_data_t::user_data_t& user_data) const;
		private:
			void delete_team(const uint64_t guild_id, const std::string& team_name) const;
			void delete_teams(const uint64_t guild_id) const;

			bool add_user_to_team(const uint64_t guild_id, const mln::mog::mog_team_data_t::user_data_t& user_data, const std::string& team_name) const;
			std::optional<mln::mog::mog_team_data_t::user_data_t> remove_user_from_team(const uint64_t guild_id, const uint64_t user_id, const std::string& team_name) const;
			bool set_team(const mln::mog::mog_team_data_t& team) const;

			void clear_teams() const;
		};
	}
}

#endif //H_MLN_DB_MOG_TEAM_H