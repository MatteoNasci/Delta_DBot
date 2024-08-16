#pragma once
#ifndef H_MLN_DB_BASE_READY_H
#define H_MLN_DB_BASE_READY_H

#include "commands/base_action.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <memory>

namespace mln {
	class base_ready : public base_action<dpp::task<void>, const dpp::ready_t&> {
	protected:
		base_ready(bot_delta* const delta);
	public:
		/**
		 * @brief base_ready is non-copyable
		 */
		base_ready(const base_ready&) = delete;

		base_ready(base_ready&&);

		/**
		 * @brief base_ready is non-copyable
		 */
		base_ready& operator=(const base_ready&) = delete;

		base_ready& operator=(base_ready&&);

		virtual bool execute_command() = 0;
	};
}

#endif //H_MLN_DB_BASE_READY_H