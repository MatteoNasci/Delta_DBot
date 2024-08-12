#pragma once
#ifndef H_MLN_DB_BUTTON_CLICK_RUNNER_H
#define H_MLN_DB_BUTTON_CLICK_RUNNER_H

#include "events/base_event.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>
#include <unordered_map>
#include <functional>

namespace mln {
    class button_click_runner final : public base_event<std::unordered_map<std::string, std::function<dpp::task<void>(const dpp::button_click_t&)>>> {
    public:
        button_click_runner();
        void attach_event() override;
    };
}

#endif //H_MLN_DB_BUTTON_CLICK_RUNNER_H