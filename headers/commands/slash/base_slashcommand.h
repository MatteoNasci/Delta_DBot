#pragma once
#ifndef H_MLN_DB_BASE_SLASHCOMMAND_H
#define H_MLN_DB_BASE_SLASHCOMMAND_H

#include "commands/base_command.h"

#include <dpp/dispatcher.h>

namespace mln {
	class base_slashcommand : public base_command<dpp::slashcommand_t>{
	protected:
		base_slashcommand(bot_delta* const delta, dpp::slashcommand&& cmd);
	public:
		base_slashcommand() = delete;
		/**
		 * @brief base_slashcommand is non-copyable
		 */
		base_slashcommand(const base_slashcommand&) = delete;

		base_slashcommand(base_slashcommand&&);

		/**
		 * @brief base_slashcommand is non-copyable
		 */
		base_slashcommand& operator=(const base_slashcommand&) = delete;

		base_slashcommand& operator=(base_slashcommand&&);
	};
}

#endif //H_MLN_DB_BASE_SLASHCOMMAND_H