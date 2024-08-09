#pragma once
#ifndef H_MLN_DB_DATABASE_CALLBACKS_H
#define H_MLN_DB_DATABASE_CALLBACKS_H

#include "database/db_column_data.h"

#include <functional>

namespace mln {
	struct database_callbacks_t {
		std::function<bool(void*)> row_callback;
		std::function<void(void*, int, db_column_data_t&&)> data_adder_callback;
		std::function<bool(int)> type_definer_callback;
		std::function<void(void*, size_t)> statement_index_callback;
		void* callback_data;

		database_callbacks_t();
		database_callbacks_t(std::function<bool(void*)>& row_callback, std::function<void(void*, int, db_column_data_t&&)>& data_adder_callback, std::function<bool(int)>& type_definer_callback, std::function<void(void*, size_t)>& statement_index_callback, void* callback_data);
	};
}

#endif //H_MLN_DB_DATABASE_CALLBACKS_H