#pragma once
#ifndef H_MLN_DB_REPORT_H
#define H_MLN_DB_REPORT_H

#include "commands/slash/base_slashcommand.h"

#include <dpp/coro/task.h>

#include <functional>
#include <optional>

namespace dpp{
    class cluster;
    struct slashcommand_t;
}

namespace mln {
    class database_handler;
    struct event_data_lite_t;

    class report final : public base_slashcommand {
    private:
        database_handler& db;
        size_t saved_insert_rep_query;
        bool valid_saved_stmt;

    public:
        report(dpp::cluster& cluster, database_handler& db);
        dpp::task<void> command(dpp::slashcommand_t event_data) const override final;

        std::optional<std::function<void()>> job(dpp::slashcommand_t event_data) const override final;
        bool use_job() const override final;
    };
}

#endif //H_MLN_DB_REPORT_H