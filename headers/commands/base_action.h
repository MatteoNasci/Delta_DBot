#pragma once
#ifndef H_MLN_DB_BASE_ACTION_H
#define H_MLN_DB_BASE_ACTION_H

namespace dpp {
	class cluster;
}

namespace mln {
	template<typename T_cmd_return, typename T_job_return, typename... T_args>
	class base_action {
	private:
		dpp::cluster& bot_cluster;
	protected:
		base_action(dpp::cluster& cluster) noexcept : bot_cluster{ cluster } {}

		dpp::cluster& bot() noexcept { return bot_cluster; }
	public:

		base_action() = delete;
		base_action(const base_action& rhs) noexcept : bot_cluster{ rhs.bot_cluster } {};
		base_action(base_action&& rhs) noexcept : bot_cluster{ rhs.bot_cluster } {};
		base_action& operator=(const base_action& rhs) noexcept {
			return *this;
		};
		base_action& operator=(base_action&& rhs) noexcept {
			return *this;
		};

		const dpp::cluster& cbot() const noexcept { return bot_cluster; };

		virtual T_cmd_return command(T_args... args) = 0;
		virtual T_job_return job(T_args... args) = 0;
		virtual bool use_job() const noexcept = 0;
	};
}

#endif //H_MLN_DB_BASE_ACTION_H