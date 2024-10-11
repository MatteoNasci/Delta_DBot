#pragma once
#ifndef H_MLN_DB_REPORT_H
#define H_MLN_DB_REPORT_H

#include "commands/slash/base_slashcommand.h"
#include "database/db_saved_stmt_state.h"

#include <dpp/coro/job.h>

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
        db_saved_stmt_state db_state;

    public:
        report(dpp::cluster& cluster, database_handler& db);
        ~report();
        report(const report&) = delete;
        report(report&& rhs) noexcept;
        report& operator=(const report&) = delete;
        report& operator=(report&& rhs) noexcept;

        dpp::job command(dpp::slashcommand_t event_data) override final;

        std::optional<std::function<void()>> job(dpp::slashcommand_t event_data) override final;
        bool use_job() const noexcept override final;
    };
}

#endif //H_MLN_DB_REPORT_H