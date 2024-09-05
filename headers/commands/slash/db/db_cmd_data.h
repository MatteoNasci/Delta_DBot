#pragma once
#ifndef H_MLN_DB_DB_CMD_DATA_H
#define H_MLN_DB_DB_CMD_DATA_H

#include <dpp/guild.h>
#include <dpp/channel.h>
#include <dpp/permissions.h>

#include <memory>

namespace mln {
	struct db_cmd_data_t {
		std::shared_ptr<const dpp::guild> cmd_guild;
		std::shared_ptr<const dpp::channel> cmd_channel;
		std::shared_ptr<const dpp::channel> dump_channel;
		std::shared_ptr<const dpp::guild_member> cmd_usr;
		std::shared_ptr<const dpp::guild_member> cmd_bot;

		dpp::permission cmd_usr_perm;
		dpp::permission cmd_bot_perm;
		dpp::permission dump_channel_bot_perm;
	};
}

#endif //H_MLN_DB_DB_CMD_DATA_H