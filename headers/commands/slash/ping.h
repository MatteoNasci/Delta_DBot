#pragma once
#ifndef H_MLN_DB_PING_H
#define H_MLN_DB_PING_H

#include "commands/slash/base_slashcommand.h"

#include <dpp/coro/job.h>

#include <functional>
#include <optional>

namespace dpp {
    class cluster;
    struct slashcommand_t;
}

namespace mln {
    class ping final : public base_slashcommand {
    public:
        ping(dpp::cluster& cluster);
        dpp::job command(dpp::slashcommand_t event_data) override final;

        std::optional<std::function<void()>> job(dpp::slashcommand_t event_data) override final;
        bool use_job() const noexcept override final;
    };
}

#endif //H_MLN_DB_PING_H