#pragma once
#ifndef H_MLN_DB_DB_INSERT_REPLACE_H
#define H_MLN_DB_DB_INSERT_REPLACE_H

#include "commands/slash/db/db_insert.h"

namespace mln {
	class bot_delta;
	class db_insert_update final : public db_insert {
	public:
		db_insert_update(bot_delta* const delta);
	};
}

#endif //H_MLN_DB_DB_INSERT_REPLACE_H