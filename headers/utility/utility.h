#pragma once
#ifndef H_MLN_DB_UTILITY_H
#define H_MLN_DB_UTILITY_H

#include <dpp/guild.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>

#include <optional>

namespace mln {
	class utility {
	public:
		static dpp::task<std::optional<dpp::guild_member>> resolve_guild_member(const dpp::interaction_create_t& event);
	};
}

#endif //H_MLN_DB_UTILITY_H