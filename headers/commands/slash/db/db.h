#pragma once
#ifndef H_MLN_DB_SHOW_H
#define H_MLN_DB_SHOW_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class db final : public base_slashcommand {
    public:
        db(bot_delta* const delta);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) override;
    };
}

#endif //H_MLN_DB_SHOW_H