#pragma once
#ifndef H_MLN_DB_DB_H
#define H_MLN_DB_DB_H

#include "commands/slash/base_slashcommand.h"
#include "commands/slash/db/base_db_command.h"

#include <dpp/coro/job.h>

#include <array>
#include <functional>
#include <memory>
#include <optional>

namespace dpp {
    class cluster;
    struct slashcommand_t;
}

namespace mln {
    class database_handler;

    class db final : public base_slashcommand {
    private:
        std::array<std::unique_ptr<mln::base_db_command>, 8> commands;

        database_handler& database;
    public:
        db(dpp::cluster& cluster, database_handler& database);
        dpp::job command(dpp::slashcommand_t event_data) override final;

        std::optional<std::function<void()>> job(dpp::slashcommand_t event_data) override final;
        bool use_job() const noexcept override final;
    };
}

#endif //H_MLN_DB_DB_H