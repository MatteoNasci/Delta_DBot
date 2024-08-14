#pragma once
#ifndef H_MLN_DB_DATABASE_CALLBACKS_H
#define H_MLN_DB_DATABASE_CALLBACKS_H

#include "database/db_column_data.h"

#include <functional>

typedef std::function<void(void*)> row_callback_f;
typedef std::function<void(void*, int, mln::db_column_data_t&&)> data_adder_callback_f;
typedef std::function<bool(void*, int)> type_definer_callback_f;
typedef std::function<void(void*, size_t)> statement_index_callback_f;
namespace mln {
	struct database_callbacks_t {
		row_callback_f row_callback;
		data_adder_callback_f data_adder_callback;
		type_definer_callback_f type_definer_callback;
		statement_index_callback_f statement_index_callback;
		void* callback_data;

		database_callbacks_t();
		database_callbacks_t(const row_callback_f& row_callback, const data_adder_callback_f& data_adder_callback, const type_definer_callback_f& type_definer_callback, const statement_index_callback_f& statement_index_callback, void* const callback_data);
	};
}

#endif //H_MLN_DB_DATABASE_CALLBACKS_H