#pragma once
#ifndef H_MLN_DB_AVATAR_H
#define H_MLN_DB_AVATAR_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class avatar final : public base_slashcommand {
    public:
        avatar(bot_delta* const delta);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) override;
    };
}

#endif //H_MLN_DB_AVATAR_H