#pragma once
#ifndef H_MLN_DB_ADD_ROLE_H
#define H_MLN_DB_ADD_ROLE_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class add_role final : public base_slashcommand {
    public:
        add_role(bot_delta* const delta);
        dpp::job command(dpp::slashcommand_t event_data) override;
    };
}

#endif //H_MLN_DB_ADD_ROLE_H