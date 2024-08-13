#pragma once
#ifndef H_MLN_DB_BASE_ACTION_H
#define H_MLN_DB_BASE_ACTION_H

#include <dpp/coro/job.h>

#include <string>

namespace mln {
	class bot_delta;

	template<typename T_interaction_create_t, typename... T_args>
	class base_action {
	private:
		bot_delta* delta_ptr;
	protected:
		base_action(bot_delta* const in_delta) : delta_ptr(in_delta) {}

		bot_delta* const delta() const { return delta_ptr; }

	public:

		base_action() = delete;

		base_action(const base_action&) = default;

		base_action(base_action&& rhs) = default;

		base_action& operator=(const base_action&) = default;

		base_action& operator=(base_action&& rhs) = default;

		virtual dpp::job command(T_interaction_create_t event, T_args... args) = 0;
	};
}

#endif //H_MLN_DB_BASE_ACTION_H