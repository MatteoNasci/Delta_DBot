#pragma once
#ifndef H_MLN_DB_MOG_HELP_H
#define H_MLN_DB_MOG_HELP_H

#include "commands/slash/mog/base_mog_command.h"
#include "commands/slash/mog/mog_command_type.h"
#include "commands/slash/mog/mog_init_type_flag.h"
#include "database/db_saved_stmt_state.h"

#include <dpp/coro/task.h>

namespace dpp {
	class cluster;
	struct slashcommand_t;
	struct message;
}

namespace mln {
	namespace mog {
		struct mog_cmd_data_t;

		class mog_help : public base_mog_command {
		private:
			static const dpp::message s_info;
		public:
			mog_help(dpp::cluster& cluster);

			dpp::task<void> command(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data, const mog_command_type type) override final;
			mog_init_type_flag get_requested_initialization_type(const mog_command_type cmd) const noexcept override final;
			db_saved_stmt_state is_db_initialized() const noexcept override final;
		};
	}
}

#endif //H_MLN_DB_MOG_HELP_H