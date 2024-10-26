#pragma once
#ifndef H_MLN_DB_BASE_EVENT_H
#define H_MLN_DB_BASE_EVENT_H

namespace dpp {
	class cluster;
}

namespace mln {
	class database_handler;
	class jobs_runner;

	template<typename T_container>
	class base_event {
	protected:
		T_container actions;
	private:
		dpp::cluster& cluster;
		database_handler& db;
		jobs_runner& j_runner;
	public:
		base_event(dpp::cluster& in_cluster, database_handler& in_db, jobs_runner& in_j_runner) : actions{}, cluster{ in_cluster }, db{ in_db }, j_runner{ in_j_runner } {};

		/**
		 * @brief base_event is non-copyable
		 */
		base_event(const base_event&) = delete;

		base_event(base_event&& rhs) : actions{ std::move(rhs.actions) }, cluster{ rhs.cluster }, db{ rhs.db }, j_runner{ rhs.j_runner } {
			rhs.actions = {};
		}

		/**
		 * @brief base_event is non-copyable
		 */
		base_event& operator=(const base_event&) = delete;

		base_event& operator=(base_event&& rhs) {
			if (this != &rhs) {
				actions = std::move(rhs.actions);
				rhs.actions = {};
			}

			return *this;
		}

		virtual void attach_event() = 0;

		const T_container& get_actions() const { return actions; }
		dpp::cluster& bot() { return cluster; }
		const dpp::cluster& cbot() const { return cluster; }
		database_handler& database() { return db; }
		jobs_runner& jobs_handler() { return j_runner; }
	};
}

#endif //H_MLN_DB_BASE_EVENT_H