#pragma once
#ifndef H_MLN_DB_HELP_H
#define H_MLN_DB_HELP_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class help final : public base_slashcommand {
    public:
        help(dpp::cluster& cluster);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) const override;
    };
}

#endif //H_MLN_DB_HELP_H