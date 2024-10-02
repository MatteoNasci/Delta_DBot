#pragma once
#ifndef H_MLN_DB_BASE_COMMAND_H
#define H_MLN_DB_BASE_COMMAND_H

#include "commands/base_action.h"

#include <dpp/appcommand.h>
#include <dpp/coro/job.h>

#include <functional>
#include <optional>
#include <type_traits>

namespace dpp {
	class cluster;
}

namespace mln {
	template<typename T_interaction_create_t>
	class base_command : public base_action<dpp::job, std::optional<std::function<void()>>, T_interaction_create_t> {
	private:
		dpp::slashcommand cmd;
	protected:
		base_command(dpp::cluster& cluster, dpp::slashcommand&& in_cmd) : base_action<dpp::job, std::optional<std::function<void()>>, T_interaction_create_t>{ cluster }, cmd{ std::forward<dpp::slashcommand>(in_cmd) } {}
	public:
		base_command() = delete;

		base_command(const base_command&) = default;

		base_command(base_command&& rhs) = default;

		base_command& operator=(const base_command&) = default;

		base_command& operator=(base_command&& rhs) = default;

		const dpp::slashcommand& get_cmd() const { return cmd; }
	};
}

#endif //H_MLN_DB_BASE_COMMAND_H