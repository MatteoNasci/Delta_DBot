#include "database/database_handler.h"
#include "database/db_prepare_flag.h"
#include "database/db_fundamental_datatype.h"
#include "database/db_config_option.h"
#include "database/db_status_param.h"

#include "sqlite3.h"

#include <unordered_map>

mln::database_callbacks_t::database_callbacks_t(std::function<bool(void*)>& in_row_callback, std::function<void(void*, int, mln::db_column_data_t&&)>& in_data_adder_callback, std::function<bool(int)>& in_type_definer_callback, void* in_callback_data) : 
	row_callback(in_row_callback), data_adder_callback(in_data_adder_callback), type_definer_callback(in_type_definer_callback), callback_data(in_callback_data) {}

mln::db_column_data_t::db_column_data_t(const char* in_name, const double in_data, const int in_bytes) : name(in_name), data(in_data), bytes(in_bytes) {}
mln::db_column_data_t::db_column_data_t(const char* in_name, const int in_data, const int in_bytes) : name(in_name), data(in_data), bytes(in_bytes) {}
mln::db_column_data_t::db_column_data_t(const char* in_name, const int64_t in_data, const int in_bytes) : name(in_name), data(in_data), bytes(in_bytes) {}
mln::db_column_data_t::db_column_data_t(const char* in_name, const void* in_data, const int in_bytes) : name(in_name), data(in_data), bytes(in_bytes) {}
mln::db_column_data_t::db_column_data_t(const char* in_name, const unsigned char* in_data, const int in_bytes) : name(in_name), data(in_data), bytes(in_bytes) {}
mln::db_column_data_t::db_column_data_t(const char* in_name, const short* in_data, const int in_bytes) : name(in_name), data(in_data), bytes(in_bytes) {}

static const std::unordered_map<mln::db_result, std::string> s_result_to_string_map{
	{mln::db_result::ok, std::string("ok")},
	{mln::db_result::error, std::string("error")},
	{mln::db_result::internal, std::string("internal")},
	{mln::db_result::perm, std::string("perm")},
	{mln::db_result::abort, std::string("abort")},
	{mln::db_result::busy, std::string("busy")},
	{mln::db_result::locked, std::string("locked")},
	{mln::db_result::no_mem, std::string("no_mem")},
	{mln::db_result::read_only, std::string("read_only")},
	{mln::db_result::interrupt, std::string("interrupt")},
	{mln::db_result::io_err, std::string("io_err")},
	{mln::db_result::corrupt, std::string("corrupt")},
	{mln::db_result::not_found, std::string("not_found")},
	{mln::db_result::full, std::string("full")},
	{mln::db_result::cant_open, std::string("cant_open")},
	{mln::db_result::protocol, std::string("protocol")},
	{mln::db_result::empty, std::string("empty")},
	{mln::db_result::schema, std::string("schema")},
	{mln::db_result::too_big, std::string("too_big")},
	{mln::db_result::constraint, std::string("constraint")},
	{mln::db_result::mismatch, std::string("mismatch")},
	{mln::db_result::misuse, std::string("misuse")},
	{mln::db_result::no_lfs, std::string("no_lfs")},
	{mln::db_result::auth, std::string("auth")},
	{mln::db_result::format, std::string("format")},
	{mln::db_result::range, std::string("range")},
	{mln::db_result::not_a_db, std::string("not_a_db")},
	{mln::db_result::notice, std::string("notice")},
	{mln::db_result::warning, std::string("warning")},
	{mln::db_result::row, std::string("row")},
	{mln::db_result::done, std::string("done")},
	{mln::db_result::ok_load_permanently, std::string("ok_load_permanently")},
	{mln::db_result::error_missing_coll_seq, std::string("error_missing_coll_seq")},
	{mln::db_result::busy_recovery, std::string("busy_recovery")},
	{mln::db_result::locked_shared_cache, std::string("locked_shared_cache")},
	{mln::db_result::read_only_recovery, std::string("read_only_recovery")},
	{mln::db_result::io_err_read, std::string("io_err_read")},
	{mln::db_result::corrupt_vtab, std::string("corrupt_vtab")},
	{mln::db_result::cant_open_no_temp_dir, std::string("cant_open_no_temp_dir")},
	{mln::db_result::constraint_check, std::string("constraint_check")},
	{mln::db_result::auth_user, std::string("auth_user")},
	{mln::db_result::notice_recover_wal, std::string("notice_recover_wal")},
	{mln::db_result::warning_auto_index, std::string("warning_auto_index")},
	{mln::db_result::error_entry, std::string("error_entry")},
	{mln::db_result::abort_rollback, std::string("abort_rollback")},
	{mln::db_result::busy_snapshot, std::string("busy_snapshot")},
	{mln::db_result::locked_vtab, std::string("locked_vtab")},
	{mln::db_result::read_only_cant_lock, std::string("read_only_cant_lock")},
	{mln::db_result::io_err_short_read, std::string("io_err_short_read")},
	{mln::db_result::corrupt_sequence, std::string("corrupt_sequence")},
	{mln::db_result::cant_open_is_dir, std::string("cant_open_is_dir")},
	{mln::db_result::constraint_commit_hook, std::string("constraint_commit_hook")},
	{mln::db_result::notice_recover_rollback, std::string("notice_recover_rollback")},
	{mln::db_result::error_snapshot, std::string("error_snapshot")},
	{mln::db_result::busy_timeout, std::string("busy_timeout")},
	{mln::db_result::read_only_rollback, std::string("read_only_rollback")},
	{mln::db_result::io_err_write, std::string("io_err_write")},
	{mln::db_result::corrupt_index, std::string("corrupt_index")},
	{mln::db_result::cant_open_full_path, std::string("cant_open_full_path")},
	{mln::db_result::constraint_foreign_key, std::string("constraint_foreign_key")},
	{mln::db_result::read_only_db_moved, std::string("read_only_db_moved")},
	{mln::db_result::io_err_fsync, std::string("io_err_fsync")},
	{mln::db_result::cant_open_conv_path, std::string("cant_open_conv_path")},
	{mln::db_result::constraint_function, std::string("constraint_function")},
	{mln::db_result::read_only_cant_init, std::string("read_only_cant_init")},
	{mln::db_result::io_err_dir_fsync, std::string("io_err_dir_fsync")},
	{mln::db_result::cant_open_dirty_wal, std::string("cant_open_dirty_wal")},
	{mln::db_result::constraint_not_null, std::string("constraint_not_null")},
	{mln::db_result::read_only_directory, std::string("read_only_directory")},
	{mln::db_result::io_err_truncate, std::string("io_err_truncate")},
	{mln::db_result::cant_open_sym_link, std::string("cant_open_sym_link")},
	{mln::db_result::constraint_primary_key, std::string("constraint_primary_key")},
	{mln::db_result::io_err_fstat, std::string("io_err_fstat")},
	{mln::db_result::constraint_trigger, std::string("constraint_trigger")},
	{mln::db_result::io_err_unlock, std::string("io_err_unlock")},
	{mln::db_result::constraint_unique, std::string("constraint_unique")},
	{mln::db_result::io_err_rd_lock, std::string("io_err_rd_lock")},
	{mln::db_result::constraint_vtab, std::string("constraint_vtab")},
	{mln::db_result::io_err_delete, std::string("io_err_delete")},
	{mln::db_result::constraint_row_id, std::string("constraint_row_id")},
	{mln::db_result::io_err_blocked, std::string("io_err_blocked")},
	{mln::db_result::constraint_pinned, std::string("constraint_pinned")},
	{mln::db_result::io_err_no_mem, std::string("io_err_no_mem")},
	{mln::db_result::constraint_data_type, std::string("constraint_data_type")},
	{mln::db_result::io_err_access, std::string("io_err_access")},
	{mln::db_result::io_err_check_reserved_lock, std::string("io_err_check_reserved_lock")},
	{mln::db_result::io_err_lock, std::string("io_err_lock")},
	{mln::db_result::io_err_close, std::string("io_err_close")},
	{mln::db_result::io_err_dir_close, std::string("io_err_dir_close")},
	{mln::db_result::io_err_shm_open, std::string("io_err_shm_open")},
	{mln::db_result::io_err_shm_size, std::string("io_err_shm_size")},
	{mln::db_result::io_err_shm_lock, std::string("io_err_shm_lock")},
	{mln::db_result::io_err_shm_map, std::string("io_err_shm_map")},
	{mln::db_result::io_err_seek, std::string("io_err_seek")},
	{mln::db_result::io_err_delete_no_ent, std::string("io_err_delete_no_ent")},
	{mln::db_result::io_err_mmap, std::string("io_err_mmap")},
	{mln::db_result::io_err_get_temp_path, std::string("io_err_get_temp_path")},
	{mln::db_result::io_err_conv_path, std::string("io_err_conv_path")},
	{mln::db_result::io_err_vnode, std::string("io_err_vnode")},
	{mln::db_result::io_err_auth, std::string("io_err_auth")},
	{mln::db_result::io_err_begin_atomic, std::string("io_err_begin_atomic")},
	{mln::db_result::io_err_commit_atomic, std::string("io_err_commit_atomic")},
	{mln::db_result::io_err_rollback_atomic, std::string("io_err_rollback_atomic")},
	{mln::db_result::io_err_data, std::string("io_err_data")},
	{mln::db_result::io_err_corrupt_fs, std::string("io_err_corrupt_fs")}
};

typedef std::function<mln::db_column_data_t(sqlite3_stmt*, int)> get_column_value;
static const std::unordered_map<mln::db_fundamental_datatype, get_column_value> s_mapped_column_funcs{
	{mln::db_fundamental_datatype::blob_t, [](sqlite3_stmt* stmt, int col) {
												//_blob and _bytes func need to be done in this order to make sure their values are correct, if _bytes happens before the value will not be correct
												const void* data = sqlite3_column_blob(stmt, col);
												const int bytes = sqlite3_column_bytes(stmt, col);
												return mln::db_column_data_t(sqlite3_column_name(stmt, col), data, bytes);
											} },
	{mln::db_fundamental_datatype::float_t, [](sqlite3_stmt* stmt, int col) {
												const double data = sqlite3_column_double(stmt, col);
												return mln::db_column_data_t(sqlite3_column_name(stmt, col), data);
											} },
	{mln::db_fundamental_datatype::integer_t, [](sqlite3_stmt* stmt, int col) {
												const int data = sqlite3_column_int(stmt, col);
												return mln::db_column_data_t(sqlite3_column_name(stmt, col), data);
											} },
	{mln::db_fundamental_datatype::text_t, [](sqlite3_stmt* stmt, int col) {
												//_blob and _bytes func need to be done in this order to make sure their values are correct, if _bytes happens before the value will not be correct
												const unsigned char* data = sqlite3_column_text(stmt, col);
												const int bytes = sqlite3_column_bytes(stmt, col);
												return mln::db_column_data_t(sqlite3_column_name(stmt, col), data, bytes);
											} },
	{mln::db_fundamental_datatype::null_t, [](sqlite3_stmt* stmt, int col) {
												return mln::db_column_data_t(sqlite3_column_name(stmt, col));
											} },
	{mln::db_fundamental_datatype::custom_int64_t, [](sqlite3_stmt* stmt, int col) {
												const int64_t data = sqlite3_column_int64(stmt, col);
												return mln::db_column_data_t(sqlite3_column_name(stmt, col), data);
											} },
	{mln::db_fundamental_datatype::custom_text16_t, [](sqlite3_stmt* stmt, int col) {
												//_blob and _bytes func need to be done in this order to make sure their values are correct, if _bytes happens before the value will not be correct
												const void* data = sqlite3_column_text16(stmt, col);
												const int bytes = sqlite3_column_bytes16(stmt, col);
												return mln::db_column_data_t(sqlite3_column_name(stmt, col), data, bytes);
											} }
};

static const std::unordered_map<mln::db_fundamental_datatype, mln::db_fundamental_datatype> s_mapped_norm_to_wide_types{
	{mln::db_fundamental_datatype::integer_t, mln::db_fundamental_datatype::custom_int64_t},
	{mln::db_fundamental_datatype::text_t, mln::db_fundamental_datatype::custom_text16_t},
	{mln::db_fundamental_datatype::blob_t, mln::db_fundamental_datatype::blob_t},
	{mln::db_fundamental_datatype::float_t, mln::db_fundamental_datatype::float_t},
	{mln::db_fundamental_datatype::null_t, mln::db_fundamental_datatype::null_t},
	{mln::db_fundamental_datatype::custom_int64_t, mln::db_fundamental_datatype::custom_int64_t},
	{mln::db_fundamental_datatype::custom_text16_t, mln::db_fundamental_datatype::custom_text16_t},
};
static const std::unordered_map<mln::db_fundamental_datatype, mln::db_fundamental_datatype> s_mapped_wide_to_norm_types{
	{mln::db_fundamental_datatype::integer_t, mln::db_fundamental_datatype::integer_t},
	{mln::db_fundamental_datatype::text_t, mln::db_fundamental_datatype::text_t},
	{mln::db_fundamental_datatype::blob_t, mln::db_fundamental_datatype::blob_t},
	{mln::db_fundamental_datatype::float_t, mln::db_fundamental_datatype::float_t},
	{mln::db_fundamental_datatype::null_t, mln::db_fundamental_datatype::null_t},
	{mln::db_fundamental_datatype::custom_int64_t, mln::db_fundamental_datatype::integer_t},
	{mln::db_fundamental_datatype::custom_text16_t, mln::db_fundamental_datatype::text_t},
};

static const std::unordered_map<mln::db_destructor_behavior, void(*)(void*)> s_mapped_destructor_behaviors{
	{mln::db_destructor_behavior::static_b, SQLITE_STATIC},
	{mln::db_destructor_behavior::transient_b, SQLITE_TRANSIENT},
	{mln::db_destructor_behavior::free_b, [](void* data) { free(data); }},
	{mln::db_destructor_behavior::delete_b, [](void* data) { delete static_cast<unsigned char*>(data); }},
	{mln::db_destructor_behavior::sqlite_free_b, [](void* data) { sqlite3_free(data); }},
};

mln::database_handler::~database_handler() {
	if (is_connected()) {
		close_connection();
	}
}

mln::database_handler::database_handler() : db(nullptr), saved_statements() {

}

mln::db_result mln::database_handler::open_connection(const char* filename, const mln::db_flag open_flags){
	if (is_connected()) {
		return mln::db_result::error;
	}

	const mln::db_result res = static_cast<mln::db_result>(sqlite3_open_v2(filename, &db, static_cast<int>(open_flags), nullptr));
	if (res != mln::db_result::ok) {
		mln::database_handler::close_connection();
	}
	return res;
}
mln::db_result mln::database_handler::open_connection(const std::string& filename, const mln::db_flag open_flags) {
	return mln::database_handler::open_connection(filename.c_str(), open_flags);
}

bool mln::database_handler::is_connected(){
	return db != nullptr;
}

mln::db_result mln::database_handler::close_connection(){
	for (const auto& stmt : saved_statements) {
		const mln::db_result res = static_cast<mln::db_result>(sqlite3_finalize(stmt));
		if (res != mln::db_result::ok) {
			return res;
		}
	}
	saved_statements.clear();

	const mln::db_result res = static_cast<mln::db_result>(sqlite3_close(db));
	if (res == mln::db_result::ok) {
		db = nullptr;
	}
	return res;
}

mln::db_result mln::database_handler::exec(const std::string& stmt, const database_callbacks_t& callbacks) {
	return mln::database_handler::exec(stmt.c_str(), static_cast<int>(stmt.length()) + 1, callbacks);
}
mln::db_result mln::database_handler::exec(const char* stmt_text, const int length_with_null, const database_callbacks_t& callbacks) {
	//TODO figure out how pzTail works and if I need it
	sqlite3_stmt* stmt(nullptr);
	mln::db_result res = static_cast<mln::db_result>(sqlite3_prepare_v3(db, stmt_text, length_with_null, static_cast<int>(mln::db_prepare_flag::none), &stmt, nullptr));
	if (res != mln::db_result::ok) {
		sqlite3_finalize(stmt);
		return res;
	}

	res = mln::database_handler::exec(stmt, callbacks);
	if (res != mln::db_result::ok) {
		sqlite3_finalize(stmt);
		return res;
	}

	return static_cast<mln::db_result>(sqlite3_finalize(stmt));
}
mln::db_result mln::database_handler::exec(const size_t saved_statement_id, const database_callbacks_t& callbacks) {
	if (saved_statement_id >= saved_statements.size()) {
		return mln::db_result::error;
	}

	return mln::database_handler::exec(saved_statements[saved_statement_id], callbacks);
}
mln::db_result mln::database_handler::exec(sqlite3_stmt* stmt, const database_callbacks_t& callbacks) {
	mln::db_result res = mln::db_result::ok;
	while (res != mln::db_result::done) {
		res = static_cast<mln::db_result>(sqlite3_step(stmt));
		if (!mln::database_handler::is_step_valid(res)) {
			//If an error occurs during _step the db will automatically run _reset on the stmt
			return res;
		}

		const int column_count = sqlite3_data_count(stmt);
		if (column_count > 0) {
			for (int i = 0; i < column_count; ++i) {
				mln::db_fundamental_datatype type = static_cast<mln::db_fundamental_datatype>(sqlite3_column_type(stmt, i));
				//these find calls are safe, column_type can only return one of the 5 fundamental types mapped on the enum, these following maps have all of them mapped out. This is safe
				type = callbacks.type_definer_callback(i) ? (s_mapped_wide_to_norm_types.find(type)->second) : (s_mapped_norm_to_wide_types.find(type)->second);
				//We know that the column_type func can only return one of the 5 (7) available types, and the map contains all of them (and they are all valid funcs). This is safe.
				const get_column_value& column_func = s_mapped_column_funcs.find(type)->second;
				callbacks.data_adder_callback(callbacks.callback_data, i, std::move(column_func(stmt, i)));
			}

			if (!callbacks.row_callback(callbacks.callback_data)) {
				sqlite3_reset(stmt);
				res = mln::db_result::abort;
				break;
			}
		}
	}

	//This is to return the last error (if any). If an error occurred in step the statement will be automatically reset
	if (res != mln::db_result::ok && res != mln::db_result::done) {
		return res;
	}

	//this will prompt sqlite to free all allocated resources (like strings) in this execution
	return static_cast<mln::db_result>(sqlite3_reset(stmt));
}

mln::db_result mln::database_handler::save_statement(const std::string& statement, size_t& out_saved_statement_id) {
	return mln::database_handler::save_statement(statement.c_str(), static_cast<int>(statement.length()) + 1, out_saved_statement_id);
}
mln::db_result mln::database_handler::save_statement(const char* statement, const int length_with_null, size_t& out_saved_statement_id) {
	//TODO figure out how pzTail works and if I need it
	sqlite3_stmt* stmt(nullptr);
	const mln::db_result res = static_cast<mln::db_result>(sqlite3_prepare_v3(db, statement, length_with_null, static_cast<int>(mln::db_prepare_flag::prepare_persistent), &stmt, nullptr));
	if (res == mln::db_result::ok) {
		out_saved_statement_id = saved_statements.size();
		saved_statements.push_back(stmt);
	}else {
		sqlite3_finalize(stmt);
	}
	
	return res;
}

mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const int param_index, const int value){
	if (saved_statement_id >= saved_statements.size()) {
		return mln::db_result::error;
	}

	sqlite3_stmt* stmt = saved_statements[saved_statement_id];
	return static_cast<mln::db_result>(sqlite3_bind_int(stmt, param_index, value));
}
mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const int param_index, const int64_t value) {
	if (saved_statement_id >= saved_statements.size()) {
		return mln::db_result::error;
	}

	sqlite3_stmt* stmt = saved_statements[saved_statement_id];
	return static_cast<mln::db_result>(sqlite3_bind_int64(stmt, param_index, value));
}
mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const int param_index, const double value) {
	if (saved_statement_id >= saved_statements.size()) {
		return mln::db_result::error;
	}

	sqlite3_stmt* stmt = saved_statements[saved_statement_id];
	return static_cast<mln::db_result>(sqlite3_bind_double(stmt, param_index, value));
}
mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const int param_index) {
	if (saved_statement_id >= saved_statements.size()) {
		return mln::db_result::error;
	}

	sqlite3_stmt* stmt = saved_statements[saved_statement_id];
	return static_cast<mln::db_result>(sqlite3_bind_null(stmt, param_index));
}
mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const int param_index, const char* text, const uint64_t bytes, const db_destructor_behavior mem_management, const db_text_encoding encoding){
	if (saved_statement_id >= saved_statements.size()) {
		return mln::db_result::error;
	}

	sqlite3_stmt* stmt = saved_statements[saved_statement_id];
	return static_cast<mln::db_result>(sqlite3_bind_text64(stmt, param_index, text, bytes, s_mapped_destructor_behaviors.find(mem_management)->second, static_cast<unsigned char>(encoding)));
}
mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const int param_index, const void* blob, const uint64_t bytes, const db_destructor_behavior mem_management) {
	if (saved_statement_id >= saved_statements.size()) {
		return mln::db_result::error;
	}

	sqlite3_stmt* stmt = saved_statements[saved_statement_id];
	return static_cast<mln::db_result>(sqlite3_bind_blob64(stmt, param_index, blob, bytes, s_mapped_destructor_behaviors.find(mem_management)->second));
}
mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const int param_index, const uint64_t bytes) {
	if (saved_statement_id >= saved_statements.size()) {
		return mln::db_result::error;
	}

	sqlite3_stmt* stmt = saved_statements[saved_statement_id];
	return static_cast<mln::db_result>(sqlite3_bind_zeroblob64(stmt, param_index, bytes));
}

std::string mln::database_handler::get_db_debug_info() {
	int64_t soft_heap_limit = sqlite3_soft_heap_limit64(-1);
	int64_t hard_heap_limit = sqlite3_hard_heap_limit64(-1);

	int64_t used_memory = sqlite3_memory_used();
	int64_t max_used_memory = sqlite3_memory_highwater(0);

	std::string debug_text("Soft heap limit: " + std::to_string(soft_heap_limit) + ", hard heap limit: " + std::to_string(hard_heap_limit) +
							"\nUsed memory: " + std::to_string(used_memory) + ", max used memory: " + std::to_string(max_used_memory));

	int64_t current = 0;
	int64_t highwater = 0;
	if (static_cast<mln::db_result>(sqlite3_status64(static_cast<int>(mln::db_status_param::memory_used), &current, &highwater, 0)) == mln::db_result::ok) {
		debug_text += "\nUsed memory: " + std::to_string(current) + ", max used memory: " + std::to_string(highwater);
	}else {
		debug_text += "\nData about memory use could not be loaded!";
	}

	current = 0;
	highwater = 0;
	if (static_cast<mln::db_result>(sqlite3_status64(static_cast<int>(mln::db_status_param::malloc_count), &current, &highwater, 0)) == mln::db_result::ok) {
		debug_text += "\nSqlite malloc count: " + std::to_string(current) + ", max sqlite malloc count: " + std::to_string(highwater);
	}else {
		debug_text += "\nData about malloc count could not be loaded!";
	}

	current = 0;
	highwater = 0;
	if (static_cast<mln::db_result>(sqlite3_status64(static_cast<int>(mln::db_status_param::malloc_size), &current, &highwater, 0)) == mln::db_result::ok) {
		debug_text += "\nSqlite malloc size: " + std::to_string(highwater);
	}else {
		debug_text += "\nData about malloc_size could not be loaded!";
	}

	current = 0;
	highwater = 0;
	if (static_cast<mln::db_result>(sqlite3_status64(static_cast<int>(mln::db_status_param::page_cache_used), &current, &highwater, 0)) == mln::db_result::ok) {
		debug_text += "\nPage cache used: " + std::to_string(current) + ", max page cache used: " + std::to_string(highwater);
	}else {
		debug_text += "\nData about page cache used could not be loaded!";
	}

	current = 0;
	highwater = 0;
	if (static_cast<mln::db_result>(sqlite3_status64(static_cast<int>(mln::db_status_param::page_cache_size), &current, &highwater, 0)) == mln::db_result::ok) {
		debug_text += "\nPage cache size: " + std::to_string(highwater);
	}else {
		debug_text += "\nData about page cache size could not be loaded!";
	}

	current = 0;
	highwater = 0;
	if (static_cast<mln::db_result>(sqlite3_status64(static_cast<int>(mln::db_status_param::page_cache_overflow), &current, &highwater, 0)) == mln::db_result::ok) {
		debug_text += "\nPage cache overflow: " + std::to_string(current) + ", max page cache overflow: " + std::to_string(highwater);
	}else {
		debug_text += "\nData about page cache overflow could not be loaded!";
	}

	current = 0;
	highwater = 0;
	if (static_cast<mln::db_result>(sqlite3_status64(static_cast<int>(mln::db_status_param::parser_stack), &current, &highwater, 0)) == mln::db_result::ok) {
		debug_text += "\nDeepest parser stack: " + std::to_string(highwater);
	}else {
		debug_text += "\nData about parser stack could not be loaded!";
	}

	return debug_text;
}

bool mln::database_handler::get_name_from_result(const mln::db_result result, std::string& out_string){
	const auto& it = s_result_to_string_map.find(result);
	const bool success = it != s_result_to_string_map.end();

	if (success) {
		out_string = it->second;
	}

	return success;
}

mln::db_result mln::database_handler::initialize_db_environment(){
	mln::db_result res = static_cast<mln::db_result>(sqlite3_config(static_cast<int>(mln::db_config_option::config_mem_status), 1));
	if (res != mln::db_result::ok) {
		return res;
	}

	res = static_cast<mln::db_result>(sqlite3_config(static_cast<int>(mln::db_config_option::config_covering_index_scan), 1));
	if (res != mln::db_result::ok) {
		return res;
	}

	res = static_cast<mln::db_result>(sqlite3_config(static_cast<int>(mln::db_config_option::config_serialized)));
	if (res != mln::db_result::ok) {
		return res;
	}

	int boolean = 1;
	res = static_cast<mln::db_result>(sqlite3_config(static_cast<int>(mln::db_config_option::config_row_id_in_view), &boolean));
	if (res != mln::db_result::ok) {
		return res;
	}

	res = static_cast<mln::db_result>(sqlite3_config(static_cast<int>(mln::db_config_option::config_small_malloc), 1));
	if (res != mln::db_result::ok) {
		return res;
	}

	res = static_cast<mln::db_result>(sqlite3_config(static_cast<int>(mln::db_config_option::config_uri), 0));
	if (res != mln::db_result::ok) {
		return res;
	}

	return static_cast<mln::db_result>(sqlite3_initialize());
}

mln::db_result mln::database_handler::shutdown_db_environment() {
	return static_cast<mln::db_result>(sqlite3_shutdown());
}

bool mln::database_handler::is_step_valid(const mln::db_result result) {
	return result == mln::db_result::row || result == mln::db_result::done || result == mln::db_result::ok;
}