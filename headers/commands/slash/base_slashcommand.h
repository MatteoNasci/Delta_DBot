#pragma once
#ifndef H_MLN_DB_BASE_SLASHCOMMAND_H
#define H_MLN_DB_BASE_SLASHCOMMAND_H

#include "commands/base_command.h"

#include <dpp/dispatcher.h>

namespace mln {
	class base_slashcommand : public base_command<dpp::slashcommand_t>{
	protected:
		base_slashcommand(dpp::cluster& cluster, dpp::slashcommand&& cmd);
	public:

		base_slashcommand() = delete;

		base_slashcommand(const base_slashcommand&) = default;

		base_slashcommand(base_slashcommand&&) = default;

		base_slashcommand& operator=(const base_slashcommand&) = default;

		base_slashcommand& operator=(base_slashcommand&&) = default;
	};
}

#endif //H_MLN_DB_BASE_SLASHCOMMAND_H