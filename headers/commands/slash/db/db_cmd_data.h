#pragma once
#ifndef H_MLN_DB_DB_CMD_DATA_H
#define H_MLN_DB_DB_CMD_DATA_H

#include "utility/event_data_lite.h"

#include <dpp/permissions.h>

#include <memory>

namespace dpp {
	class guild;
	class channel;
	class guild_member;
}

namespace mln {

	struct db_cmd_data_t {
		mln::event_data_lite_t data;
		std::shared_ptr<const dpp::guild> cmd_guild;
		std::shared_ptr<const dpp::channel> cmd_channel;
		std::shared_ptr<const dpp::channel> dump_channel;
		std::shared_ptr<const dpp::guild_member> cmd_usr;
		std::shared_ptr<const dpp::guild_member> cmd_bot;

		dpp::permission cmd_usr_perm;
		dpp::permission cmd_bot_perm;
		dpp::permission dump_channel_bot_perm;

		db_cmd_data_t() noexcept;
	};
}

#endif //H_MLN_DB_DB_CMD_DATA_H