#pragma once
#ifndef H_MLN_DB_MSGS_GET_H
#define H_MLN_DB_MSGS_GET_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class msgs_get final : public base_slashcommand {
    public:
        msgs_get(bot_delta* const delta);
        dpp::job command(dpp::slashcommand_t event_data) override;
    };
}

#endif //H_MLN_DB_MSGS_GET_H