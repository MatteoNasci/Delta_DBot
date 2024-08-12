#pragma once
#ifndef H_MLN_DB_BOT_INFO_H
#define H_MLN_DB_BOT_INFO_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class bot_info final : public base_slashcommand {
    public:
        bot_info(bot_delta* const delta);
        dpp::job command(dpp::slashcommand_t event_data) override;
    };
}

#endif //H_MLN_DB_BOT_INFO_H