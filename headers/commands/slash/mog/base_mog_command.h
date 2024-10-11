#pragma once
#ifndef H_MLN_DB_MOG_BASE_MOG_COMMAND_H
#define H_MLN_DB_MOG_BASE_MOG_COMMAND_H

#include "commands/base_action.h"
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

		class base_mog_command : public base_action<dpp::task<void>, void, const dpp::slashcommand_t&, mln::mog::mog_cmd_data_t&, const mln::mog::mog_command_type> {
		protected:
			base_mog_command(dpp::cluster& cluster);
		public:
			base_mog_command() = delete;

			virtual mln::mog::mog_init_type_flag get_requested_initialization_type(const mln::mog::mog_command_type cmd) const noexcept = 0;
			virtual db_saved_stmt_state is_db_initialized() const noexcept = 0;
			bool use_job() const noexcept override final;
			void job(const dpp::slashcommand_t&, mln::mog::mog_cmd_data_t&, const mln::mog::mog_command_type) override final;
		};
	}
}

#endif //H_MLN_DB_MOG_BASE_MOG_COMMAND_H