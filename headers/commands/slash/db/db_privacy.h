#pragma once
#ifndef H_MLN_DB_DB_PRIVACY_H
#define H_MLN_DB_DB_PRIVACY_H

#include "commands/slash/db/base_db_command.h"

namespace mln {
	class db_privacy : public base_db_command {
	private:

	public:
		db_privacy(dpp::cluster& cluster);
		dpp::task<void> command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const db_command_type type) const override;
		db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const override;

	private:
	};
}

#endif //H_MLN_DB_DB_PRIVACY_H