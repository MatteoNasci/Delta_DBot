#pragma once
#ifndef H_MLN_DB_MOG_HELP_H
#define H_MLN_DB_MOG_HELP_H

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

		class mog_help : public base_mog_command {
		private:

		public:
			mog_help(dpp::cluster& cluster);
			dpp::task<void> command(const dpp::slashcommand_t& event_data, cmd_data_t& cmd_data, const command_type type) const override final;
			init_type_flag get_requested_initialization_type(const command_type cmd) const override final;
			bool is_db_initialized() const override final;
		private:
		};
	}
}

#endif //H_MLN_DB_MOG_HELP_H