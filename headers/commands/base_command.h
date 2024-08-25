#pragma once
#ifndef H_MLN_DB_BASE_COMMAND_H
#define H_MLN_DB_BASE_COMMAND_H

#include "commands/base_action.h"

#include <dpp/appcommand.h>
#include <dpp/coro/task.h>

namespace mln {
	template<typename T_interaction_create_t>
	class base_command : public base_action<dpp::task<void>, const T_interaction_create_t&> {
	private:
		dpp::slashcommand cmd;
	protected:
		base_command(bot_delta* const in_delta, dpp::slashcommand&& in_cmd) : base_action<dpp::task<void>, const T_interaction_create_t&>(in_delta), cmd(std::forward<dpp::slashcommand>(in_cmd)) {}
	public:
		base_command() = delete;

		base_command(const base_command&) = default;

		base_command(base_command&& rhs) : base_action<dpp::task<void>, const T_interaction_create_t&>(std::forward<base_command>(rhs)), cmd(std::move(rhs.cmd)) {}

		base_command& operator=(const base_command&) = default;

		base_command& operator=(base_command&& rhs) {
			this->cmd = std::move(rhs.cmd);
			base_action<dpp::task<void>, const T_interaction_create_t&>::operator=(std::forward<base_command>(rhs));
			return *this;
		}

		const dpp::slashcommand& get_command() { return cmd; };
	};
}

#endif //H_MLN_DB_BASE_COMMAND_H