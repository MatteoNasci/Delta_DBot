#include "database/database_handler.h"
#include "database/db_prepare_flag.h"
#include "database/db_fundamental_datatype.h"
#include "database/db_config_option.h"
#include "database/db_status_param.h"

#include "sqlite3.h"

mln::db_column_data_t read_data_text16(sqlite3_stmt* stmt, int col);
mln::db_column_data_t read_data_int64(sqlite3_stmt* stmt, int col);
mln::db_column_data_t read_data_null(sqlite3_stmt* stmt, int col);
mln::db_column_data_t read_data_text(sqlite3_stmt* stmt, int col);
mln::db_column_data_t read_data_blob(sqlite3_stmt* stmt, int col);
mln::db_column_data_t read_data_double(sqlite3_stmt* stmt, int col);
mln::db_column_data_t read_data_int(sqlite3_stmt* stmt, int col);

void behavior_delete(void* data);
void behavior_free(void* data);
void behavior_sqlite_free(void* data);

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

typedef std::function<mln::db_column_data_t(sqlite3_stmt*, int)> get_column_value_f;
static const std::unordered_map<mln::db_fundamental_datatype, get_column_value_f> s_mapped_column_funcs{
	{mln::db_fundamental_datatype::blob_t, read_data_blob },
	{mln::db_fundamental_datatype::float_t, read_data_double },
	{mln::db_fundamental_datatype::integer_t, read_data_int },
	{mln::db_fundamental_datatype::text_t, read_data_text },
	{mln::db_fundamental_datatype::null_t, read_data_null },
	{mln::db_fundamental_datatype::custom_int64_t, read_data_int64 },
	{mln::db_fundamental_datatype::custom_text16_t, read_data_text16 }
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

static const std::unordered_map<mln::db_destructor_behavior, void(*)(void*)> s_mapped_destructor_behaviors{
	{mln::db_destructor_behavior::static_b, SQLITE_STATIC},
	{mln::db_destructor_behavior::transient_b, SQLITE_TRANSIENT},
	{mln::db_destructor_behavior::free_b, behavior_free},
	{mln::db_destructor_behavior::delete_b, behavior_delete },
	{mln::db_destructor_behavior::sqlite_free_b, behavior_sqlite_free },
};


mln::database_callbacks_t::database_callbacks_t(const row_callback_f& in_row_callback, const data_adder_callback_f& in_data_adder_callback, const type_definer_callback_f& in_type_definer_callback, const statement_index_callback_f& in_statement_index_callback, const statement_completed_callback_f& in_statement_completed_callback, void* const in_callback_data) :
	row_callback(in_row_callback), data_adder_callback(in_data_adder_callback), type_definer_callback(in_type_definer_callback), statement_index_callback(in_statement_index_callback), statement_completed_callback(in_statement_completed_callback), callback_data(in_callback_data) {}

mln::database_callbacks_t::database_callbacks_t() : database_callbacks_t(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr){}

mln::db_column_data_t::db_column_data_t(const char* in_name, double in_data, int in_bytes) : name(in_name), data(in_data), bytes(in_bytes) {}
mln::db_column_data_t::db_column_data_t(const char* in_name, int in_data, int in_bytes) : name(in_name), data(in_data), bytes(in_bytes) {}
mln::db_column_data_t::db_column_data_t(const char* in_name, int64_t in_data, int in_bytes) : name(in_name), data(in_data), bytes(in_bytes) {}
mln::db_column_data_t::db_column_data_t(const char* in_name, const void* in_data, int in_bytes) : name(in_name), data(in_data), bytes(in_bytes) {}
mln::db_column_data_t::db_column_data_t(const char* in_name, const unsigned char* in_data, int in_bytes) : name(in_name), data(in_data), bytes(in_bytes) {}
mln::db_column_data_t::db_column_data_t(const char* in_name, const short* in_data, int in_bytes) : name(in_name), data(in_data), bytes(in_bytes) {}

mln::database_handler::~database_handler() {
	if (is_connected()) {
		close_connection();
	}
}

mln::database_handler::database_handler() : db(nullptr), removed_keys(), saved_statements() {

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

bool mln::database_handler::is_connected() const{
	return db != nullptr;
}

mln::db_result mln::database_handler::close_connection(){
	mln::database_handler::delete_all_statement();

	const mln::db_result res = static_cast<mln::db_result>(sqlite3_close(db));
	if (res == mln::db_result::ok) {
		db = nullptr;
	}
	return res;
}

mln::db_result mln::database_handler::exec(const std::string& stmt, const database_callbacks_t& callbacks) const {
	return mln::database_handler::exec(stmt.c_str(), static_cast<int>(stmt.length()), callbacks);
}
mln::db_result mln::database_handler::exec(const char* stmt_text, int length_with_null, const database_callbacks_t& callbacks) const {
	std::queue<sqlite3_stmt*> stmt_queue = {};
	const char* tail(stmt_text);
	mln::db_result res(mln::db_result::ok);

	//prepare phase

	//length_with_null represents the number of characters from the start of the statement until '\0' (the null char is also counted)
	//Sqlite3 only prepares the first statement in the string if there are multiple present in it (separated by semicolons)
	//This ugly pointer arithmetic is used to automatically update the length_with_null depending on where the _prepare function ends
	//It's safer to keep the -(tail - start) rather than unpacking to avoid problems with the int length_with_null overflowing
	//The length adapts to tail changing with _prepare, and at the end of the whole process length will be == 1 so we cycle for > 1
	//length_with_null = length_with_null - (tail - start);
	while (*tail != '\0') {
		sqlite3_stmt* stmt(nullptr);
		const char* start(tail);

		res = static_cast<mln::db_result>(sqlite3_prepare_v3(db, start, length_with_null, static_cast<int>(mln::db_prepare_flag::none), &stmt, &tail));

		if (res != mln::db_result::ok) {
			sqlite3_finalize(stmt);
			while (!stmt_queue.empty()) {
				sqlite3_finalize(stmt_queue.front());
				stmt_queue.pop();
			}
			return res;
		}

		stmt_queue.push(stmt);
		length_with_null = length_with_null - static_cast<int>(tail - start);
	}

	//execution/finalize phase
	size_t stmt_index = 0;
	const bool use_stmt_index_callback = stmt_queue.size() > 1 && callbacks.statement_index_callback;
	while (!stmt_queue.empty() && !mln::database_handler::is_exec_error(res)) {
		if (use_stmt_index_callback) {
			callbacks.statement_index_callback(callbacks.callback_data, stmt_index++);
		}

		sqlite3_stmt* stmt = stmt_queue.front();
		res = mln::database_handler::exec(stmt, callbacks);
		stmt_queue.pop();

		sqlite3_finalize(stmt);
	}

	while (!stmt_queue.empty()) {
		sqlite3_finalize(stmt_queue.front());
		stmt_queue.pop();
	}
	
	return res;
}
mln::db_result mln::database_handler::exec(const size_t saved_statement_id, const database_callbacks_t& callbacks) const {
	const auto& it = saved_statements.find(saved_statement_id);
	if (it == saved_statements.end()) {
		return mln::db_result::range;
	}

	const size_t stmt_count = it->second.size();
	const bool use_stmt_index_callback = stmt_count > 1 && callbacks.statement_index_callback;
	for (size_t i = 0; i < stmt_count; ++i) {
		if (use_stmt_index_callback) {
			callbacks.statement_index_callback(callbacks.callback_data, i);
		}

		const mln::db_result res = mln::database_handler::exec(it->second[i], callbacks);
		if (mln::database_handler::is_exec_error(res)) {
			return res;
		}
	}

	return mln::db_result::ok;
}

mln::db_result mln::database_handler::exec(sqlite3_stmt* stmt, const database_callbacks_t& callbacks) const {
	const bool can_use_callbacks = (callbacks.data_adder_callback && callbacks.type_definer_callback);
	const bool can_use_row_call = (callbacks.row_callback && true);

	mln::db_result res = mln::db_result::ok;
	while (res != mln::db_result::done) {
		res = static_cast<mln::db_result>(sqlite3_step(stmt));
		if (!mln::database_handler::is_step_valid(res)) {

			mln::db_result reset_res = static_cast<mln::db_result>(sqlite3_reset(stmt));
			return mln::database_handler::is_step_valid(reset_res) ? res : reset_res;
		}

		const int column_count = sqlite3_data_count(stmt);
		if (can_use_callbacks && column_count > 0) {
			for (int i = 0; i < column_count; ++i) {
				mln::db_fundamental_datatype type = static_cast<mln::db_fundamental_datatype>(sqlite3_column_type(stmt, i));
				//these find calls are safe, column_type can only return one of the 5 fundamental types mapped on the enum, these following maps have all of them mapped out. This is safe
				if (!callbacks.type_definer_callback(callbacks.callback_data, i)){
					type = s_mapped_norm_to_wide_types.find(type)->second;
				}
				//We know that the column_type func can only return one of the 5 (7) available types, and the map contains all of them (and they are all valid funcs). This is safe.
				const get_column_value_f& column_func = s_mapped_column_funcs.find(type)->second;
				callbacks.data_adder_callback(callbacks.callback_data, i, std::forward<mln::db_column_data_t>(column_func(stmt, i)));
			}

			if (can_use_row_call) {
				callbacks.row_callback(callbacks.callback_data);
			}
		}
	}

	if (callbacks.statement_completed_callback) {
		callbacks.statement_completed_callback(callbacks.callback_data);
	}
	//this will prompt sqlite to free all allocated resources (like strings) in this execution
	mln::db_result reset_res = static_cast<mln::db_result>(sqlite3_reset(stmt));
	return mln::database_handler::is_step_valid(reset_res) ? res : reset_res;
}

mln::db_result mln::database_handler::save_statement(const std::string& statement, size_t& out_saved_statement_id) {
	return mln::database_handler::save_statement(statement.c_str(), static_cast<int>(statement.length()), out_saved_statement_id);
}
mln::db_result mln::database_handler::save_statement(const char* statement, int length_with_null, size_t& out_saved_statement_id) {
	std::vector<sqlite3_stmt*>* list(nullptr);
	const char* tail(statement);
	mln::db_result res(mln::db_result::ok);

	//length_with_null represents the number of characters from the start of the statement until '\0' (the null char is also counted)
	//Sqlite3 only prepares the first statement in the string if there are multiple present in it (separated by semicolons)
	//This ugly pointer arithmetic is used to automatically update the length_with_null depending on where the _prepare function ends
	//It's safer to keep the -(tail - start) rather than unpacking to avoid problems with the int length_with_null overflowing
	//The length adapts to tail changing with _prepare, and at the end of the whole process length will be == 1 so we cycle for > 1
	//length_with_null = length_with_null - (tail - start);
	while (*tail != '\0') {
		sqlite3_stmt* stmt(nullptr);
		const char* start(tail);
		res = static_cast<mln::db_result>(sqlite3_prepare_v3(db, start, length_with_null, static_cast<int>(mln::db_prepare_flag::prepare_persistent), &stmt, &tail));

		if (res != mln::db_result::ok) {
			//If we have already created the saved statement we make sure to delete it
			if (list != nullptr) {
				mln::database_handler::delete_statement(out_saved_statement_id);
			}
			sqlite3_finalize(stmt);
			return res;
		}

		if (list == nullptr) {
			//This function will always return an id that is not present in the map
			out_saved_statement_id = mln::database_handler::get_available_key();

			list = &(saved_statements.insert(std::make_pair(out_saved_statement_id, std::vector<sqlite3_stmt*>())).first->second);
		}
		list->push_back(stmt);

		length_with_null = length_with_null - static_cast<int>(tail - start);
	}
	
	return res;
}
mln::db_result mln::database_handler::delete_statement(const size_t saved_statement_id) {
	const auto& it = saved_statements.find(saved_statement_id);
	if (it == saved_statements.end()) {
		return mln::db_result::range;
	}

	const size_t stmt_count = it->second.size();
	for (size_t i = 0; i < stmt_count; ++i) {
		sqlite3_finalize(it->second[i]);
	}

	saved_statements.erase(it);
	removed_keys.push(saved_statement_id);

	return mln::db_result::ok;
}
void mln::database_handler::delete_all_statement() {
	for (const std::pair<size_t, std::vector<sqlite3_stmt*>>& pair : saved_statements) {
		for (sqlite3_stmt* stmt : pair.second) {
			sqlite3_finalize(stmt);
		}
	}
	saved_statements.clear();

	while (!removed_keys.empty()) {
		removed_keys.pop();
	}
}

bool mln::database_handler::is_saved_stmt_id_valid(const size_t saved_statement_id) const {
	return saved_statements.contains(saved_statement_id);
}
size_t mln::database_handler::get_available_key() {
	size_t res;
	if(removed_keys.size() > 0) {
		res = removed_keys.front();
		removed_keys.pop();
	}
	else {
		res = saved_statements.size();
	}

	return res;
}

mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const size_t stmt_index, const int param_index, const int value) const {
	const auto& it = saved_statements.find(saved_statement_id);
	if (it == saved_statements.end()) {
		return mln::db_result::range;
	}

	if (stmt_index >= it->second.size()) {
		return mln::db_result::range;
	}

	return static_cast<mln::db_result>(sqlite3_bind_int(it->second[stmt_index], param_index, value));
}

mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const size_t stmt_index, const int param_index, const int64_t value) const {
	const auto& it = saved_statements.find(saved_statement_id);
	if (it == saved_statements.end()) {
		return mln::db_result::range;
	}

	if (stmt_index >= it->second.size()) {
		return mln::db_result::range;
	}

	return static_cast<mln::db_result>(sqlite3_bind_int64(it->second[stmt_index], param_index, value));
}
mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const size_t stmt_index, const int param_index, const double value) const {
	const auto& it = saved_statements.find(saved_statement_id);
	if (it == saved_statements.end()) {
		return mln::db_result::range;
	}

	if (stmt_index >= it->second.size()) {
		return mln::db_result::range;
	}

	return static_cast<mln::db_result>(sqlite3_bind_double(it->second[stmt_index], param_index, value));
}
mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const size_t stmt_index, const int param_index) const {
	const auto& it = saved_statements.find(saved_statement_id);
	if (it == saved_statements.end()) {
		return mln::db_result::range;
	}

	if (stmt_index >= it->second.size()) {
		return mln::db_result::range;
	}

	return static_cast<mln::db_result>(sqlite3_bind_null(it->second[stmt_index], param_index));
}
mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const size_t stmt_index, const int param_index, const char* text, const uint64_t bytes, const db_destructor_behavior mem_management, const db_text_encoding encoding) const {
	const auto& it = saved_statements.find(saved_statement_id);
	if (it == saved_statements.end()) {
		return mln::db_result::range;
	}

	if (stmt_index >= it->second.size()) {
		return mln::db_result::range;
	}

	return static_cast<mln::db_result>(sqlite3_bind_text64(it->second[stmt_index], param_index, text, bytes, s_mapped_destructor_behaviors.find(mem_management)->second, static_cast<unsigned char>(encoding)));
}
mln::db_result mln::database_handler::bind_parameter(size_t saved_statement_id, size_t stmt_index, int param_index, const std::string& text, db_text_encoding encoding) const {
	const auto& it = saved_statements.find(saved_statement_id);
	if (it == saved_statements.end()) {
		return mln::db_result::range;
	}

	if (stmt_index >= it->second.size()) {
		return mln::db_result::range;
	}

	return static_cast<mln::db_result>(sqlite3_bind_text64(it->second[stmt_index], param_index, text.c_str(), text.length(), s_mapped_destructor_behaviors.find(mln::db_destructor_behavior::transient_b)->second, static_cast<unsigned char>(encoding)));
}
mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const size_t stmt_index, const int param_index, const void* blob, const uint64_t bytes, const db_destructor_behavior mem_management) const {
	const auto& it = saved_statements.find(saved_statement_id);
	if (it == saved_statements.end()) {
		return mln::db_result::range;
	}

	if (stmt_index >= it->second.size()) {
		return mln::db_result::range;
	}

	return static_cast<mln::db_result>(sqlite3_bind_blob64(it->second[stmt_index], param_index, blob, bytes, s_mapped_destructor_behaviors.find(mem_management)->second));
}
mln::db_result mln::database_handler::bind_parameter(const size_t saved_statement_id, const size_t stmt_index, const int param_index, const void*, const uint64_t bytes) const {
	const auto& it = saved_statements.find(saved_statement_id);
	if (it == saved_statements.end()) {
		return mln::db_result::range;
	}

	if (stmt_index >= it->second.size()) {
		return mln::db_result::range;
	}

	return static_cast<mln::db_result>(sqlite3_bind_zeroblob64(it->second[stmt_index], param_index, bytes));
}
mln::db_result mln::database_handler::get_bind_parameter_index(const size_t saved_statement_id, const size_t stmt_index, const char* param_name, int& out_index) const {
	const auto& it = saved_statements.find(saved_statement_id);
	if (it == saved_statements.end()) {
		return mln::db_result::range;
	}

	if (stmt_index >= it->second.size()) {
		return mln::db_result::range;
	}

	out_index = sqlite3_bind_parameter_index(it->second[stmt_index], param_name);
	return out_index == 0 ? mln::db_result::error : mln::db_result::ok;
}

std::string mln::database_handler::get_last_err_msg() const{
	const char* err = sqlite3_errmsg(db);
	return std::string(err == nullptr ? "Error not found" : err);
}

size_t mln::database_handler::get_last_changes() const {
	return static_cast<size_t>(sqlite3_changes64(db));
}
int64_t mln::database_handler::get_last_insert_rowid() const {
	return static_cast<int64_t>(sqlite3_last_insert_rowid(db));
}

std::string mln::database_handler::get_db_debug_info() {
	int64_t soft_heap_limit = sqlite3_soft_heap_limit64(-1);
	int64_t hard_heap_limit = sqlite3_hard_heap_limit64(-1);

	std::string debug_text("Soft heap limit: " + std::to_string(soft_heap_limit) + ", hard heap limit: " + std::to_string(hard_heap_limit));

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
std::string mln::database_handler::get_name_from_result(db_result result) {
	std::string s;
	if (!mln::database_handler::get_name_from_result(result, s)) {
		s = "unknown db error";
	}
	return s;
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

	res = static_cast<mln::db_result>(sqlite3_config(static_cast<int>(mln::db_config_option::config_small_malloc), 0));
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
bool mln::database_handler::is_exec_error(const mln::db_result result) {
	return result != mln::db_result::row && result != mln::db_result::done && result != mln::db_result::ok;
}

mln::db_column_data_t read_data_int(sqlite3_stmt* stmt, int col) {
	const int data = sqlite3_column_int(stmt, col);
	return mln::db_column_data_t(sqlite3_column_name(stmt, col), data);
}
mln::db_column_data_t read_data_double(sqlite3_stmt* stmt, int col) {
	const double data = sqlite3_column_double(stmt, col);
	return mln::db_column_data_t(sqlite3_column_name(stmt, col), data);
}
mln::db_column_data_t read_data_blob(sqlite3_stmt* stmt, int col) {
	//_blob and _bytes func need to be done in this order to make sure their values are correct, if _bytes happens before the value will not be correct
	const void* data = sqlite3_column_blob(stmt, col);
	const int bytes = sqlite3_column_bytes(stmt, col);
	return mln::db_column_data_t(sqlite3_column_name(stmt, col), data, bytes);
}
mln::db_column_data_t read_data_text(sqlite3_stmt* stmt, int col) {
	//_blob and _bytes func need to be done in this order to make sure their values are correct, if _bytes happens before the value will not be correct
	const unsigned char* data = sqlite3_column_text(stmt, col);
	const int bytes = sqlite3_column_bytes(stmt, col);
	return mln::db_column_data_t(sqlite3_column_name(stmt, col), data, bytes);
}
mln::db_column_data_t read_data_null(sqlite3_stmt* stmt, int col) {
	return mln::db_column_data_t(sqlite3_column_name(stmt, col));
}
mln::db_column_data_t read_data_int64(sqlite3_stmt* stmt, int col) {
	const int64_t data = sqlite3_column_int64(stmt, col);
	return mln::db_column_data_t(sqlite3_column_name(stmt, col), data);
}
mln::db_column_data_t read_data_text16(sqlite3_stmt* stmt, int col) {
	//_blob and _bytes func need to be done in this order to make sure their values are correct, if _bytes happens before the value will not be correct
	const void* data = sqlite3_column_text16(stmt, col);
	const int bytes = sqlite3_column_bytes16(stmt, col);
	return mln::db_column_data_t(sqlite3_column_name(stmt, col), data, bytes);
}

void behavior_free(void* data) {
	free(data);
}
void behavior_sqlite_free(void* data) {
	sqlite3_free(data);
}
void behavior_delete(void* data) {
	delete static_cast<unsigned char*>(data);
}