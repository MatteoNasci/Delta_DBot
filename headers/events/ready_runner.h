#pragma once
#ifndef H_MLN_DB_READY_RUNNER_H
#define H_MLN_DB_READY_RUNNER_H

#include "events/base_event.h"
#include "commands/ready/base_ready.h"

#include <vector>

namespace mln {
    class ready_runner final : public base_event<std::vector<std::unique_ptr<base_ready>>> {
    public:
        void attach_event(bot_delta* const delta) override;
    };
}

#endif //H_MLN_DB_READY_RUNNER_H