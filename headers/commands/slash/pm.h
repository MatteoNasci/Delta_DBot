#pragma once
#ifndef H_MLN_DB_PM_H
#define H_MLN_DB_PM_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class pm final : public base_slashcommand {
    public:
        pm(dpp::cluster& cluster);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) const override;
    };
}

#endif //H_MLN_DB_PM_H