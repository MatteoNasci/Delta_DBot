#pragma once
#ifndef H_MLN_DB_BASE_EVENT_H
#define H_MLN_DB_BASE_EVENT_H

#include <utility>

namespace mln {
	class bot_delta;
	template<typename T_container>
	class base_event {
	protected:
		T_container actions;

	public:
		base_event() : actions() {};

		/**
		 * @brief base_event is non-copyable
		 */
		base_event(const base_event&) = delete;

		base_event(base_event&& rhs) : actions(std::forward<base_event>(rhs.actions)) {}

		/**
		 * @brief base_event is non-copyable
		 */
		base_event& operator=(const base_event&) = delete;

		base_event& operator=(base_event&& rhs) {
			this->actions = std::forward<base_event>(rhs.actions);
			return *this;
		}

		virtual void attach_event(bot_delta* const delta) = 0;
		const T_container& get_actions() const { return actions; }
	};
}

#endif //H_MLN_DB_BASE_EVENT_H