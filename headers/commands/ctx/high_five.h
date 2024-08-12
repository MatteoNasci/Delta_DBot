#pragma once
#ifndef H_MLN_DB_HIGH_FIVE_H
#define H_MLN_DB_HIGH_FIVE_H

#include "commands/ctx/base_ctx_command.h"

namespace mln {
    class high_five final : public base_ctx_command{
	public:
		high_five(bot_delta* const delta);
		dpp::job command(dpp::user_context_menu_t event_data) override;
    };
}

#endif //H_MLN_DB_HIGH_FIVE_H