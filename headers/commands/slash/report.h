#pragma once
#ifndef H_MLN_DB_REPORT_H
#define H_MLN_DB_REPORT_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class report final : public base_slashcommand {
    public:
        report(bot_delta* const delta);
        dpp::job command(dpp::slashcommand_t event_data) override;
    };
}

#endif //H_MLN_DB_REPORT_H