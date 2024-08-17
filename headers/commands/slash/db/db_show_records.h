#pragma once
#ifndef H_MLN_DB_DB_SHOW_RECORDS_H
#define H_MLN_DB_DB_SHOW_RECORDS_H

#include "commands/slash/db/base_db_command.h"

namespace mln {
	class db_show_records final : public base_db_command {
	private:
		size_t saved_stmt, saved_verbose_stmt;
		bool valid_stmt;
	public:
		db_show_records(bot_delta* const delta);
		dpp::task<void> command(const dpp::command_data_option&, const dpp::slashcommand_t& event_data, url_type) override;
	};
}

#endif //H_MLN_DB_DB_SHOW_RECORDS_H