#pragma once
#ifndef H_MLN_DB_BASE_DB_COMMAND_H
#define H_MLN_DB_BASE_DB_COMMAND_H

#include "commands/base_action.h"
#include "commands/slash/db/url_type.h"

#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/appcommand.h>

namespace mln {
	class bot_delta;
	class base_db_command : public base_action<dpp::task<void>, const dpp::command_data_option&, const dpp::slashcommand_t&, url_type> {
	protected:
		base_db_command(bot_delta* const delta);
	public:
		base_db_command() = delete;

		base_db_command(const base_db_command&) = default;

		base_db_command(base_db_command&& rhs) = default;

		base_db_command& operator=(const base_db_command&) = default;

		base_db_command& operator=(base_db_command&& rhs) = default;
	};
}

#endif //H_MLN_DB_BASE_DB_COMMAND_H