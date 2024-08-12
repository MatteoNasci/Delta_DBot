#pragma once
#ifndef H_MLN_DB_PING_H
#define H_MLN_DB_PING_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class ping final : public base_slashcommand {
    public:
        ping(bot_delta* const delta);
        dpp::job command(dpp::slashcommand_t event_data) override;
    };
}

#endif //H_MLN_DB_PING_H