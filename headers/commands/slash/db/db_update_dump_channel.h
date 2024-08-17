#pragma once
#ifndef H_MLN_DB_DB_UPDATE_DUMP_CHANNEL_H
#define H_MLN_DB_DB_UPDATE_DUMP_CHANNEL_H

#include "commands/slash/db/base_db_command.h"

namespace mln {
	class db_update_dump_channel final : public base_db_command {
	private:
		size_t saved_stmt;
		int saved_param_guild, saved_param_channel;
		bool valid_stmt;
	public:
		db_update_dump_channel(bot_delta* const delta);
		dpp::task<void> command(const dpp::command_data_option&, const dpp::slashcommand_t& event_data, url_type) override;
	};
}

#endif //H_MLN_DB_DB_UPDATE_DUMP_CHANNEL_H