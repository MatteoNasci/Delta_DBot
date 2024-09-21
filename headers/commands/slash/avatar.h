#pragma once
#ifndef H_MLN_DB_AVATAR_H
#define H_MLN_DB_AVATAR_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class avatar final : public base_slashcommand {
    public:
        avatar(dpp::cluster& cluster);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) const override;
    };
}

#endif //H_MLN_DB_AVATAR_H