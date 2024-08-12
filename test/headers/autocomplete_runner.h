#pragma once
#ifndef H_MLN_DB_AUTOCOMPLETE_RUNNER_H
#define H_MLN_DB_AUTOCOMPLETE_RUNNER_H

#include "events/base_event.h"
#include "commands/autocomplete/base_autocomplete.h"

#include <dpp/dispatcher.h>
#include <dpp/appcommand.h>
#include <dpp/coro/task.h>

#include <unordered_map>
#include <cstdint>
#include <variant>

namespace mln {
    class autocomplete_runner final : public base_event<std::unordered_map<uint64_t, std::variant<>>>{
    public:
        autocomplete_runner();
        void attach_event() override;
    };
}

#endif //H_MLN_DB_AUTOCOMPLETE_RUNNER_H