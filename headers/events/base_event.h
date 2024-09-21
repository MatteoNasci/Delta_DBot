#pragma once
#ifndef H_MLN_DB_BASE_EVENT_H
#define H_MLN_DB_BASE_EVENT_H

#include <utility>

namespace dpp {
	class cluster;
}

namespace mln {
	class database_handler;

	template<typename T_container>
	class base_event {
	protected:
		T_container actions;
	private:
		dpp::cluster& cluster;
		database_handler& db;
	public:
		base_event(dpp::cluster& in_cluster, database_handler& in_db) : actions{}, cluster{ in_cluster }, db{ in_db } {};

		/**
		 * @brief base_event is non-copyable
		 */
		base_event(const base_event&) = delete;

		base_event(base_event&& rhs) : actions(std::forward<base_event>(rhs.actions)) {}

		/**
		 * @brief base_event is non-copyable
		 */
		base_event& operator=(const base_event&) = delete;

		base_event& operator=(base_event&& rhs) {
			this->actions = std::forward<base_event>(rhs.actions);
			return *this;
		}

		virtual void attach_event() = 0;
		const T_container& get_actions() const { return actions; }
		dpp::cluster& bot() { return cluster; }
		database_handler& database() { return db; }
	};
}

#endif //H_MLN_DB_BASE_EVENT_H