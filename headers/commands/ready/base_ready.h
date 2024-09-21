#pragma once
#ifndef H_MLN_DB_BASE_READY_H
#define H_MLN_DB_BASE_READY_H

#include "commands/base_action.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

namespace mln {
	class base_ready : public base_action<dpp::task<void>, const dpp::ready_t&> {
	protected:
		base_ready(dpp::cluster& cluster);
	public:

		base_ready() = delete;

		base_ready(const base_ready&) = default;

		base_ready(base_ready&&) = default;

		base_ready& operator=(const base_ready&) = default;

		base_ready& operator=(base_ready&&) = default;
	};
}

#endif //H_MLN_DB_BASE_READY_H