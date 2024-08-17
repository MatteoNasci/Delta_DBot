#pragma once
#ifndef H_MLN_DB_DB_DELETE_H
#define H_MLN_DB_DB_DELETE_H

#include "commands/slash/db/base_db_command.h"

namespace mln {
	class db_delete final : public base_db_command {
	private:
		size_t saved_stmt;
		int saved_param_guild, saved_param_user, saved_param_name;
		bool valid_stmt;
	public:
		db_delete(bot_delta* const delta);
		dpp::task<void> command(const dpp::command_data_option&, const dpp::slashcommand_t& event_data, url_type) override;
	};
}

#endif //H_MLN_DB_DB_DELETE_H