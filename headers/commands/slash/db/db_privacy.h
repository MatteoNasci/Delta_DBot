#pragma once
#ifndef H_MLN_DB_DB_PRIVACY_H
#define H_MLN_DB_DB_PRIVACY_H

#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_init_type_flag.h"
#include "database/db_saved_stmt_state.h"

#include <dpp/coro/task.h>

namespace dpp {
	class cluster;
	struct slashcommand_t;
}

namespace mln {
	struct db_cmd_data_t;

	class db_privacy : public base_db_command {
	public:
		db_privacy(dpp::cluster& cluster);

		dpp::task<void> command(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const db_command_type type) override final;
		db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const noexcept override final;
		db_saved_stmt_state is_db_initialized() const noexcept override final;
	};
}

#endif //H_MLN_DB_DB_PRIVACY_H