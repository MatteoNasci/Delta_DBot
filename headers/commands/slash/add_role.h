#pragma once
#ifndef H_MLN_DB_ADD_ROLE_H
#define H_MLN_DB_ADD_ROLE_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class add_role final : public base_slashcommand {
    public:
        add_role(dpp::cluster& cluster);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) const override;
    };
}

#endif //H_MLN_DB_ADD_ROLE_H