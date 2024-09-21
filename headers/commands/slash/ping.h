#pragma once
#ifndef H_MLN_DB_PING_H
#define H_MLN_DB_PING_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class ping final : public base_slashcommand {
    public:
        ping(dpp::cluster& cluster);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) const override;
    };
}

#endif //H_MLN_DB_PING_H