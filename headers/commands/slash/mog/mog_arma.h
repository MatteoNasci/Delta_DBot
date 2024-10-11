#pragma once
#ifndef H_MLN_DB_MOG_ARMA_H
#define H_MLN_DB_MOG_ARMA_H

#include "commands/slash/mog/base_mog_command.h"
#include "commands/slash/mog/mog_command_type.h"
#include "commands/slash/mog/mog_init_type_flag.h"
#include "database/db_saved_stmt_state.h"

#include <dpp/coro/task.h>

namespace dpp {
	class cluster;
	struct slashcommand_t;
}

namespace mln {
	namespace mog {
		struct mog_cmd_data_t;
		class mog_team;

		class mog_arma final : public base_mog_command {
		private:
			mln::mog::mog_team& teams_handler;
		public:
			mog_arma(dpp::cluster& cluster, mln::mog::mog_team& teams_handler);

			dpp::task<void> command(const dpp::slashcommand_t& event_data, mln::mog::mog_cmd_data_t& cmd_data, const mln::mog::mog_command_type type) override final;
			mln::mog::mog_init_type_flag get_requested_initialization_type(const mln::mog::mog_command_type cmd) const noexcept override final;
			db_saved_stmt_state is_db_initialized() const noexcept override final;

		private:
			dpp::task<void> cooldown(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const;
			dpp::task<void> show_cooldowns(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const;
			dpp::task<void> help(mog_cmd_data_t& cmd_data) const;
		};
	}
}

#endif //H_MLN_DB_MOG_ARMA_H