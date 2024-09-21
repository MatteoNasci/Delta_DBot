#pragma once
#ifndef H_MLN_DB_REPORT_H
#define H_MLN_DB_REPORT_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class database_handler;

    class report final : public base_slashcommand {
    private:
        database_handler& db;
        size_t saved_insert_rep_query;
        bool valid_saved_stmt;

    public:
        report(dpp::cluster& cluster, database_handler& db);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) const override;
    };
}

#endif //H_MLN_DB_REPORT_H