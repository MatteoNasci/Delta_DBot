#pragma once
#ifndef H_MLN_DB_BASE_CTX_COMMAND_H
#define H_MLN_DB_BASE_CTX_COMMAND_H

#include "commands/base_command.h"

#include <dpp/dispatcher.h>

namespace mln {
	class base_ctx_command : public base_command<dpp::user_context_menu_t> {
	protected:
		base_ctx_command(dpp::cluster& cluster, dpp::slashcommand&& cmd);
	public:

		base_ctx_command() = delete;

		base_ctx_command(const base_ctx_command&) = default;

		base_ctx_command(base_ctx_command&&) = default;

		base_ctx_command& operator=(const base_ctx_command&) = default;

		base_ctx_command& operator=(base_ctx_command&&) = default;
	};
}

#endif //H_MLN_DB_BASE_CTX_COMMAND_H