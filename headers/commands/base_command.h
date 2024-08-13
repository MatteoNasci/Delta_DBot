#pragma once
#ifndef H_MLN_DB_BASE_COMMAND_H
#define H_MLN_DB_BASE_COMMAND_H

#include "commands/base_action.h"

#include <dpp/appcommand.h>

namespace mln {
	template<typename T_interaction_create_t>
	class base_command : public base_action<T_interaction_create_t> {
	private:
		dpp::slashcommand cmd;
	protected:
		base_command(bot_delta* const in_delta, dpp::slashcommand&& in_cmd) : base_action<T_interaction_create_t>(in_delta), cmd(std::move(in_cmd)) {}
	public:
		base_command() = delete;

		base_command(const base_command&) = default;

		base_command(base_command&& rhs) : base_action<T_interaction_create_t>(std::move(rhs)), cmd(std::move(rhs.cmd)) {}

		base_command& operator=(const base_command&) = default;

		base_command& operator=(base_command&& rhs) {
			this->cmd = std::move(rhs.cmd);
			base_action<T_interaction_create_t>::operator=(std::move(rhs));
			return *this;
		}

		const dpp::slashcommand& get_command() { return cmd; };
	};
}

#endif //H_MLN_DB_BASE_COMMAND_H