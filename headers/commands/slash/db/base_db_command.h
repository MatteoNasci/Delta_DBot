#pragma once
#ifndef H_MLN_DB_BASE_DB_COMMAND_H
#define H_MLN_DB_BASE_DB_COMMAND_H

#include "commands/base_action.h"
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

	class base_db_command : public base_action<dpp::task<void>, void, const dpp::slashcommand_t&, db_cmd_data_t&, const db_command_type> {
	protected:
		base_db_command(dpp::cluster& cluster) noexcept;
	public:
		base_db_command() = delete;

		virtual db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const noexcept = 0;
		virtual db_saved_stmt_state is_db_initialized() const noexcept = 0;
		bool use_job() const  noexcept override final;
		void job(const dpp::slashcommand_t&, db_cmd_data_t&, const db_command_type) override final;
	};
}

#endif //H_MLN_DB_BASE_DB_COMMAND_H