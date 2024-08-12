#pragma once
#ifndef H_MLN_DB_BASE_AUTOCOMPLETE_H
#define H_MLN_DB_BASE_AUTOCOMPLETE_H

#include "commands/base_action.h"

#include <dpp/appcommand.h>

namespace mln {
	class base_autocomplete : base_action<dpp::autocomplete_t, const dpp::command_option&>{
	protected:
		base_autocomplete(const std::string& cmd_name);
	public:
		base_autocomplete() = delete;
		/**
		 * @brief base_autocomplete is non-copyable
		 */
		base_autocomplete(const base_autocomplete&) = delete;

		/**
		 * @brief base_autocomplete is non-copyable
		 */
		base_autocomplete& operator=(const base_autocomplete&) = delete;
	};
}

#endif //H_MLN_DB_BASE_AUTOCOMPLETE_H