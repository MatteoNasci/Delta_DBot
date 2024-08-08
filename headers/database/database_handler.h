#pragma once
#ifndef H_MLN_DB_DATABASE_HANDLER_H
#define H_MLN_DB_DATABASE_HANDLER_H

#include "database/db_result.h"
#include "database/db_column_data.h"
#include "database/db_text_encoding.h"
#include "database/db_destructor_behavior.h"
#include "database/db_flag.h"

#include <string>
#include <vector>
#include <functional>

struct sqlite3;
struct sqlite3_stmt;
namespace mln {
	class database_handler {
	private:
		sqlite3* db;
		std::vector<sqlite3_stmt*> saved_statements;
	public:
		~database_handler();
		database_handler();
		/**
		 * @brief database_handler is non-copyable
		 */
		database_handler(const database_handler&) = delete;
		/**
		 * @brief database_handler is non-moveable
		 */
		database_handler(const database_handler&&) = delete;
		/**
		 * @brief database_handler is non-copyable
		 */
		database_handler& operator=(const database_handler&) = delete;
		/**
		 * @brief database_handler is non-moveable
		 */
		database_handler& operator=(const database_handler&&) = delete;

		db_result open_connection(const std::string& filename, const db_flag open_flags);
		db_result open_connection(const char* filename, const db_flag open_flags);
		bool is_connected();
		db_result close_connection();

		//These 2 execs are not really meant to be optimized, only to be used for one time use statements
		db_result exec(const std::string& statement, int(callback)(void*, int, char**, char**), void* callback_data, const std::function<void(const char* const)>& err_callback);
		db_result exec(const char* const statement, int(callback)(void*, int, char**, char**), void* callback_data, const std::function<void(const char* const)>& err_callback);

		//The callback_data_adder will be called for each element in the row, for each row (as long as any column is present in the result), with: first arg is callback_data, second arg is the current column index, third arg is the actual value of the column of the correct type
		//The callback_definer will be called for each element in the row, for each row (as long as any column is present in the result), with: first arg is column_index, return is a boolean that represents whenever the database should interpret the data as a normal value or as an extended value. If true the values will be normal (int as int, text as text), otherwise the values will be extended (int as int64_t, text as text16)
		//The callback will be called for each row (as long as any column is present in the result), with: first arg is callback_data, return is a boolean. If the boolean returned is false the execution will be aborted, otherwise the execution will continue as normal.
		//The callback_data should be something able to "store" the column result from the query. The result/s will be given to callback_data one at a time through callback_data_adder for each column in the resulting row (for each row). Once all the data from the current row result has been added the callback_data will be sent to callback
		//After the call to callback the callback_data should be ready to accept a new set of column results.
		db_result exec(
			const size_t saved_statement_id, 
			const std::function<bool(void*)>& callback, 
			void* callback_data, 
			const std::function<void(void*, int, db_column_data_t&&)>& callback_data_adder, 
			const std::function<bool(int)>& callback_definer);

		db_result save_statement(const std::string& statement, size_t& out_saved_statement_id);
		db_result save_statement(const char* statement, const int length_with_null, size_t& out_saved_statement_id);

		db_result bind_parameter(const size_t saved_statement_id, const int param_index, const int value);
		db_result bind_parameter(const size_t saved_statement_id, const int param_index, const int64_t value);
		db_result bind_parameter(const size_t saved_statement_id, const int param_index, const double value);
		db_result bind_parameter(const size_t saved_statement_id, const int param_index);
		db_result bind_parameter(const size_t saved_statement_id, const int param_index, const char* text, const uint64_t bytes, const db_destructor_behavior mem_management, const db_text_encoding encoding);
		db_result bind_parameter(const size_t saved_statement_id, const int param_index, const void* blob, const uint64_t bytes, const db_destructor_behavior mem_management);
		db_result bind_parameter(const size_t saved_statement_id, const int param_index, const uint64_t bytes);

		/*Value and pointer binds will not be supported at the moment*/
	private:
		static bool is_step_valid(const db_result result);
	
	public:
		static std::string get_db_debug_info();
		static bool get_name_from_result(const db_result result, std::string& out_name);

		//Threadsafe, must be called before any other db action.
		static db_result initialize_db_environment();
		//Not threadsafe, all db resources must have already been freed
		static db_result shutdown_db_environment();
		
	};
}

#endif //H_MLN_DB_DATABASE_HANDLER_H