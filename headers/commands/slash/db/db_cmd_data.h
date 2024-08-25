#pragma once
#ifndef H_MLN_DB_DB_CMD_DATA_H
#define H_MLN_DB_DB_CMD_DATA_H

#include <dpp/guild.h>
#include <dpp/channel.h>

namespace mln {
	struct db_cmd_data_t {
		dpp::guild* cmd_guild;
		dpp::channel* cmd_channel;
		dpp::channel* dump_channel;
		const dpp::guild_member* cmd_usr;
		const dpp::guild_member* cmd_bot;
	};
}

#endif //H_MLN_DB_DB_CMD_DATA_H