#pragma once
#ifndef H_MLN_DB_ADD_ROLE_H
#define H_MLN_DB_ADD_ROLE_H

#include "commands/slash/base_slashcommand.h"

#include <dpp/coro/task.h>

#include <functional>
#include <optional>

namespace dpp {
    class cluster;
    struct slashcommand_t;
}

namespace mln {
    class add_role final : public base_slashcommand {
    public:
        add_role(dpp::cluster& cluster);
        dpp::task<void> command(dpp::slashcommand_t event_data) const override final;

        std::optional<std::function<void()>> job(dpp::slashcommand_t event_data) const override final;
        bool use_job() const override final;
    };
}

#endif //H_MLN_DB_ADD_ROLE_H