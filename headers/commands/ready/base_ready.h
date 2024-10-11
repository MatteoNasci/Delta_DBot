#pragma once
#ifndef H_MLN_DB_BASE_READY_H
#define H_MLN_DB_BASE_READY_H

#include "commands/base_action.h"

#include <dpp/coro/task.h>

#include <functional>
#include <optional>

namespace dpp {
	class cluster;
	struct ready_t;
}

namespace mln {
	class base_ready : public base_action<dpp::task<void>, std::optional<std::function<void()>>, const dpp::ready_t&> {
	protected:
		base_ready(dpp::cluster& cluster) noexcept;
	public:

		base_ready() = delete;
	};
}

#endif //H_MLN_DB_BASE_READY_H