#pragma once
#ifndef H_MLN_DB_DB_SAVED_STMT_STATE_H
#define H_MLN_DB_DB_SAVED_STMT_STATE_H

namespace mln {
	enum class db_saved_stmt_state : unsigned int {
		none = 0,
		stmt_initialized = 1 << 0,
		params_initialized = 1 << 1,
		initialized = (1 << 0) | (1 << 1),
	};

	extern const char* get_saved_stmt_state_text(const mln::db_saved_stmt_state type) noexcept;
}

#endif //H_MLN_DB_DB_SAVED_STMT_STATE_H