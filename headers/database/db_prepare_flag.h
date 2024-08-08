#pragma once
#ifndef H_MLN_DB_DB_PREPARE_FLAG_H
#define H_MLN_DB_DB_PREPARE_FLAG_H

namespace mln {
	enum class db_prepare_flag {
		none = 0,
		/*The SQLITE_PREPARE_PERSISTENT flag is a hint to the query planner that the prepared statement will be retained for a long time and probably reused many times. Without this flag, sqlite3_prepare_v3() and sqlite3_prepare16_v3() assume that the prepared statement will be used just once or at most a few times and then destroyed using sqlite3_finalize() relatively soon. The current implementation acts on this hint by avoiding the use of lookaside memory so as not to deplete the limited store of lookaside memory. Future versions of SQLite may act on this hint differently.*/
		prepare_persistent = 0x01,
		/*The SQLITE_PREPARE_NORMALIZE flag is a no-op. This flag used to be required for any prepared statement that wanted to use the sqlite3_normalized_sql() interface. However, the sqlite3_normalized_sql() interface is now available to all prepared statements, regardless of whether or not they use this flag.*/
		prepare_normalize = 0x02,
		/*The SQLITE_PREPARE_NO_VTAB flag causes the SQL compiler to return an error (error code SQLITE_ERROR) if the statement uses any virtual tables.*/
		prepare_no_vtab = 0x04,
	};
}

#endif //H_MLN_DB_DB_PREPARE_FLAG_H