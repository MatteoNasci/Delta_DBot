#pragma once
#ifndef H_MLN_DB_DATABASE_HANDLER_H
#define H_MLN_DB_DATABASE_HANDLER_H

#include "database/db_result.h"

#include <string>

struct sqlite3;
namespace mln {
	class database_handler {
	private:
		static sqlite3* db;
	public:
		static std::string get_db_debug_info();
		static db_result open_connection(const std::string& filename);
		static db_result is_connected();
		static db_result close_connection();

	};
}

#endif //H_MLN_DB_DATABASE_HANDLER_H