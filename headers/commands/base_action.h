#pragma once
#ifndef H_MLN_DB_BASE_ACTION_H
#define H_MLN_DB_BASE_ACTION_H

namespace dpp {
	class cluster;
}

namespace mln {
	template<typename T_return, typename... T_args>
	class base_action {
	private:
		dpp::cluster& bot_cluster;
	protected:
		base_action(dpp::cluster& cluster) : bot_cluster{ cluster } {}

		dpp::cluster& bot() const { return bot_cluster; }
	public:

		base_action() = delete;

		base_action(const base_action&) = default;

		base_action(base_action&& rhs) = default;

		base_action& operator=(const base_action&) = default;

		base_action& operator=(base_action&& rhs) = default;

		virtual T_return command(T_args... args) const = 0;
	};
}

#endif //H_MLN_DB_BASE_ACTION_H