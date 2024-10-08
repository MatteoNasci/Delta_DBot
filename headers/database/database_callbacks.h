#pragma once
#ifndef H_MLN_DB_DATABASE_CALLBACKS_H
#define H_MLN_DB_DATABASE_CALLBACKS_H

#include "database/db_column_data.h"

#include <functional>

typedef std::function<void(void*)> row_callback_f;
typedef std::function<void(void*, int, mln::db_column_data_t&&)> data_adder_callback_f;
typedef std::function<bool(void*, int)> type_definer_callback_f;
typedef std::function<void(void*, size_t)> statement_index_callback_f;
typedef std::function<void(void*)> statement_completed_callback_f;
namespace mln {
	struct database_callbacks_t {
		row_callback_f row_callback;
		data_adder_callback_f data_adder_callback;
		type_definer_callback_f type_definer_callback;
		statement_index_callback_f statement_index_callback;
		statement_completed_callback_f statement_completed_callback;
		void* callback_data;

		database_callbacks_t();
		database_callbacks_t(const row_callback_f& row_callback, const data_adder_callback_f& data_adder_callback, const type_definer_callback_f& type_definer_callback, const statement_index_callback_f& statement_index_callback, const statement_completed_callback_f& statement_completed_callback, void* const callback_data);
	};
}
//The statement_index_callback will be called to identify which statement we are executing (in case of multiple statements in the query), indexed from 0. If only one statement is present the callback will not be used.
//The data_adder_callback will be called for each element in the row, for each row (as long as any column is present in the result), with: first arg is callback_data, second arg is the current column index, third arg is the actual value of the column of the correct type
//The type_definer_callback will be called for each element in the row, for each row (as long as any column is present in the result), with: first arg is column_index, return is a boolean that represents whenever the database should interpret the data as a normal value or as an extended value. If true the values will be normal (int as int, text as text), otherwise the values will be extended (int as int64_t, text as text16)
//The row_callback will be called for each row (as long as any column is present in the result), with: first arg is callback_data, return is a boolean. If the boolean returned is false the execution will be aborted, otherwise the execution will continue as normal.
//The callback_data should be something able to "store" the column result from the query. The result/s will be given to callback_data one at a time through data_adder_callback for each column in the resulting row (for each row). Once all the data from the current row result has been added the callback_data will be sent to row_callback
//After the call to row_callback the callback_data should be ready to accept a new set of column results or to receive a statement_index_callback callback with a new index (in case of multiple statements in the query).

#endif //H_MLN_DB_DATABASE_CALLBACKS_H