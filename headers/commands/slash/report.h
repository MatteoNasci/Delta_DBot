#pragma once
#ifndef H_MLN_DB_REPORT_H
#define H_MLN_DB_REPORT_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class bot_delta;
    class report final : public base_slashcommand {
    private:
        size_t saved_insert_rep_query;
        int guild_param_index;
        int user_param_index;
        int rep_text_param_index;
        bool valid_saved_stmt;

    public:
        report(bot_delta* const delta);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) override;
    };
}

#endif //H_MLN_DB_REPORT_H