#pragma once
#ifndef H_MLN_DB_BASE_GUILD_CREATE_H
#define H_MLN_DB_BASE_GUILD_CREATE_H

#include "commands/base_action.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <memory>

namespace mln {
	class base_guild_create : public base_action<dpp::task<void>, const dpp::guild_create_t&> {
	protected:
		base_guild_create(bot_delta* const delta);
	public:
		/**
		 * @brief base_guild_create is non-copyable
		 */
		base_guild_create(const base_guild_create&) = delete;

		base_guild_create(base_guild_create&&);

		/**
		 * @brief base_guild_create is non-copyable
		 */
		base_guild_create& operator=(const base_guild_create&) = delete;

		base_guild_create& operator=(base_guild_create&&);
	};
}

#endif //H_MLN_DB_BASE_GUILD_CREATE_H