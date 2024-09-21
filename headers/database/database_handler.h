#pragma once
#ifndef H_MLN_DB_DATABASE_HANDLER_H
#define H_MLN_DB_DATABASE_HANDLER_H

#include "database/db_result.h"
#include "database/database_callbacks.h"
#include "database/db_text_encoding.h"
#include "database/db_destructor_behavior.h"
#include "database/db_flag.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <queue>

struct sqlite3;
struct sqlite3_stmt;
namespace mln {
	class database_handler {
	private:
		sqlite3* db;
		std::queue<size_t> removed_keys;
		std::unordered_map<size_t, std::vector<sqlite3_stmt*>> saved_statements;
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
		database_handler(database_handler&&) = delete;
		/**
		 * @brief database_handler is non-copyable
		 */
		database_handler& operator=(const database_handler&) = delete;
		/**
		 * @brief database_handler is non-moveable
		 */
		database_handler& operator=(database_handler&&) = delete;

		db_result_t open_connection(const std::string& filename, db_flag open_flags = db_flag::open_rw_create_ex_res_code);
		db_result_t open_connection(const char* filename, db_flag open_flags = db_flag::open_rw_create_ex_res_code);
		bool is_connected() const;
		db_result_t close_connection();

		//The statement_index_callback will be called to identify which statement we are executing (in case of multiple statements in the query), indexed from 0. If only one statement is present the callback will not be used.
		//The data_adder_callback will be called for each element in the row, for each row (as long as any column is present in the result), with: first arg is callback_data, second arg is the current column index, third arg is the actual value of the column of the correct type
		//The type_definer_callback will be called for each element in the row, for each row (as long as any column is present in the result), with: first arg is column_index, return is a boolean that represents whenever the database should interpret the data as a normal value or as an extended value. If true the values will be normal (int as int, text as text), otherwise the values will be extended (int as int64_t, text as text16)
		//The row_callback will be called for each row (as long as any column is present in the result), with: first arg is callback_data, return is a boolean. If the boolean returned is false the execution will be aborted, otherwise the execution will continue as normal.
		//The callback_data should be something able to "store" the column result from the query. The result/s will be given to callback_data one at a time through data_adder_callback for each column in the resulting row (for each row). Once all the data from the current row result has been added the callback_data will be sent to row_callback
		//After the call to row_callback the callback_data should be ready to accept a new set of column results or to receive a statement_index_callback callback with a new index (in case of multiple statements in the query).
		db_result_t exec(
			size_t saved_statement_id, 
			const database_callbacks_t& callbacks) const;
		//The saved_id overload is to be preferred for statements that will be executed several times
		db_result_t exec(
			const std::string& stmt,
			const database_callbacks_t& callbacks) const;
		//The saved_id overload is to be preferred for statements that will be executed several times
		db_result_t exec(
			const char* stmt,
			int length_with_null,
			const database_callbacks_t& callbacks) const;

		db_result_t save_statement(const std::string& statement, size_t& out_saved_statement_id);
		db_result_t save_statement(const char* statement, int length_with_null, size_t& out_saved_statement_id);
		//Use only when you know you don't need the statement anymore
		db_result_t delete_statement(size_t saved_statement_id);

		void delete_all_statement();

		bool is_saved_stmt_id_valid(size_t saved_statement_id) const;

		//The bind param_index are indexed from 1!
		//https://www.sqlite.org/capi3ref.html#sqlite3_bind_blob
		db_result_t bind_parameter(size_t saved_statement_id, size_t stmt_index, int param_index, int value) const;
		db_result_t bind_parameter(size_t saved_statement_id, size_t stmt_index, int param_index, int64_t value) const;
		db_result_t bind_parameter(size_t saved_statement_id, size_t stmt_index, int param_index, double value) const;
		db_result_t bind_parameter(size_t saved_statement_id, size_t stmt_index, int param_index) const;
		db_result_t bind_parameter(size_t saved_statement_id, size_t stmt_index, int param_index, const char* text, uint64_t bytes, db_destructor_behavior mem_management, db_text_encoding encoding) const;
		db_result_t bind_parameter(size_t saved_statement_id, size_t stmt_index, int param_index, const std::string& text, db_text_encoding encoding) const;
		db_result_t bind_parameter(size_t saved_statement_id, size_t stmt_index, int param_index, const void* blob, uint64_t bytes, db_destructor_behavior mem_management) const;
		//The void* is ignored here
		db_result_t bind_parameter(size_t saved_statement_id, size_t stmt_index, int param_index, const void*, uint64_t bytes) const;

		db_result_t get_bind_parameter_index(size_t saved_statement_id, size_t stmt_index, const char* param_name, int& out_index) const;

		std::string get_last_err_msg() const;
		size_t get_last_changes() const;
		int64_t get_last_insert_rowid() const;

		/*Value and pointer binds will not be supported at the moment*/
	private:
		db_result_t exec(
			sqlite3_stmt* stmt,
			const database_callbacks_t& callbacks) const;

		size_t get_available_key();
	private:
		static bool is_step_valid(db_result result);
		
	public:
		static bool is_exec_error(db_result result);
		static std::string get_db_debug_info();
		static bool get_name_from_result(db_result result, std::string& out_name);
		static std::string get_name_from_result(db_result result);

		//Threadsafe, must be called before any other db action.
		static db_result_t initialize_db_environment();
		//Not threadsafe, all db resources must have already been freed
		static db_result_t shutdown_db_environment();
		
	};
}

#endif //H_MLN_DB_DATABASE_HANDLER_H