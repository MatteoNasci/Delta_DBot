#pragma once
#ifndef H_MLN_DB_PING_H
#define H_MLN_DB_PING_H

#include "commands/slash/base_slashcommand.h"

#include <dpp/coro/task.h>

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
        dpp::task<void> command(dpp::slashcommand_t event_data) const override final;

        std::optional<std::function<void()>> job(dpp::slashcommand_t event_data) const override final;
        bool use_job() const override final;
    };
}

#endif //H_MLN_DB_PING_H