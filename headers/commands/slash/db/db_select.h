#pragma once
#ifndef H_MLN_DB_DB_SELECT_H
#define H_MLN_DB_DB_SELECT_H

#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/url_type.h"

namespace mln {
	class db_select final : public base_db_command {
	private:
		size_t saved_stmt, saved_verbose_stmt;
		int saved_param_guild, saved_param_name;
		int saved_param_verbose_guild, saved_param_verbose_name;
		bool valid_stmt;

		struct record_data_t{
			std::string url, url_type, desc, usr, time;
			mln::url_type type;
			bool verbose, found;
		};
	public:
		db_select(bot_delta* const delta);
		dpp::task<void> command(const dpp::command_data_option&, const dpp::slashcommand_t& event_data, url_type type) override;
	};
}

#endif //H_MLN_DB_DB_SELECT_H