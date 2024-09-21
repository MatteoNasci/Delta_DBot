#pragma once
#ifndef H_MLN_DB_BOT_INFO_H
#define H_MLN_DB_BOT_INFO_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class bot_info final : public base_slashcommand {
    public:
        bot_info(dpp::cluster& cluster);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) const override;
    };
}

#endif //H_MLN_DB_BOT_INFO_H