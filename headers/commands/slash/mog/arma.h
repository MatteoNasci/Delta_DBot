#pragma once
#ifndef H_MLN_DB_MOG_ARMA_H
#define H_MLN_DB_MOG_ARMA_H

#include "commands/slash/mog/base_mog_command.h"
#include "commands/slash/mog/command_type.h"
#include "commands/slash/mog/init_type_flag.h"

#include <dpp/coro/task.h>

namespace dpp {
	class cluster;
	struct slashcommand_t;
}

namespace mln {
	namespace mog {
		struct cmd_data_t;
		class team;

		class arma final : public base_mog_command {
		private:
			mln::mog::team& teams_handler;
		public:
			arma(dpp::cluster& cluster, mln::mog::team& teams_handler);
			dpp::task<void> command(const dpp::slashcommand_t& event_data, mln::mog::cmd_data_t& cmd_data, const mln::mog::command_type type) const override final;
			mln::mog::init_type_flag get_requested_initialization_type(const mln::mog::command_type cmd) const override final;
			bool is_db_initialized() const override final;

		private:
			dpp::task<void> cooldown(const dpp::slashcommand_t& event_data, cmd_data_t& cmd_data) const;
			dpp::task<void> show_cooldowns(const dpp::slashcommand_t& event_data, cmd_data_t& cmd_data) const;
			dpp::task<void> help(cmd_data_t& cmd_data) const;
		};
	}
}

#endif //H_MLN_DB_MOG_ARMA_H