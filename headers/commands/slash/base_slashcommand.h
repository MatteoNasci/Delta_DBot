#pragma once
#ifndef H_MLN_DB_BASE_SLASHCOMMAND_H
#define H_MLN_DB_BASE_SLASHCOMMAND_H

#include "commands/base_command.h"

namespace dpp {
	class cluster;
	struct slashcommand_t;
	class slashcommand;
	struct message;
}

namespace mln {
	struct event_data_lite_t;

	class base_slashcommand : public base_command<dpp::slashcommand_t>{
	protected:
		base_slashcommand(dpp::cluster& cluster, dpp::slashcommand&& cmd);

		void log_incorrect_command() const;
	public:

		base_slashcommand() = delete;
	};
}

#endif //H_MLN_DB_BASE_SLASHCOMMAND_H