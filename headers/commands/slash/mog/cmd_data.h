#pragma once
#ifndef H_MLN_DB_MOG_CMD_DATA_H
#define H_MLN_DB_MOG_CMD_DATA_H

#include "utility/event_data_lite.h"

#include <dpp/permissions.h>

#include <memory>

namespace dpp {
	class guild;
	class channel;
	class guild_member;
}

namespace mln {
    namespace mog {
        struct cmd_data_t {
			mln::event_data_lite_t data;
			std::shared_ptr<const dpp::guild> cmd_guild;
			std::shared_ptr<const dpp::channel> cmd_channel;
			std::shared_ptr<const dpp::guild_member> cmd_usr;
			std::shared_ptr<const dpp::guild_member> cmd_bot;

			dpp::permission cmd_usr_perm;
			dpp::permission cmd_bot_perm;
        };
    }
}

#endif //H_MLN_DB_MOG_CMD_DATA_H