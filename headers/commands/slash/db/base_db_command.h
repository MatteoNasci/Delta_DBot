#pragma once
#ifndef H_MLN_DB_BASE_DB_COMMAND_H
#define H_MLN_DB_BASE_DB_COMMAND_H

#include "commands/base_action.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_init_type_flag.h"

#include <dpp/coro/task.h>

namespace dpp {
	class cluster;
	struct slashcommand_t;
}

namespace mln {
	struct db_cmd_data_t;

	class base_db_command : public base_action<dpp::task<void>, void, const dpp::slashcommand_t&, db_cmd_data_t&, const db_command_type> {
	protected:
		base_db_command(dpp::cluster& cluster);
	public:
		base_db_command() = delete;

		base_db_command(const base_db_command&) = default;

		base_db_command(base_db_command&& rhs) = default;

		base_db_command& operator=(const base_db_command&) = default;

		base_db_command& operator=(base_db_command&& rhs) = default;

		virtual db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const = 0;
		virtual bool is_db_initialized() const = 0;
		bool use_job() const override final;
		void job(const dpp::slashcommand_t&, db_cmd_data_t&, const db_command_type) const override final;
	};
}

#endif //H_MLN_DB_BASE_DB_COMMAND_H