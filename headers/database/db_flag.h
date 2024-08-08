#pragma once
#ifndef H_MLN_DB_DB_FLAG_H
#define H_MLN_DB_DB_FLAG_H

namespace mln {
	enum class db_flag {
		open_read_only = 0x00000001,  /* Ok for sqlite3_open_v2() */
		open_read_write = 0x00000002,  /* Ok for sqlite3_open_v2() */
		open_create = 0x00000004,  /* Ok for sqlite3_open_v2() */
		open_delete_on_close = 0x00000008,  /* VFS only */
		open_exclusive = 0x00000010,  /* VFS only */
		open_auto_proxy = 0x00000020,  /* VFS only */
		open_uri = 0x00000040,  /* Ok for sqlite3_open_v2() */
		open_memory = 0x00000080,  /* Ok for sqlite3_open_v2() */
		open_main_db = 0x00000100,  /* VFS only */
		open_temp_db = 0x00000200,  /* VFS only */
		open_transient_db = 0x00000400,  /* VFS only */
		open_main_journal = 0x00000800,  /* VFS only */
		open_temp_journal = 0x00001000,  /* VFS only */
		open_sub_journal = 0x00002000,  /* VFS only */
		open_super_journal = 0x00004000,  /* VFS only */
		open_no_mutex = 0x00008000,  /* Ok for sqlite3_open_v2() */
		open_full_mutex = 0x00010000,  /* Ok for sqlite3_open_v2() */
		open_shared_cache = 0x00020000,  /* Ok for sqlite3_open_v2() */
		open_private_cache = 0x00040000,  /* Ok for sqlite3_open_v2() */
		open_wal = 0x00080000,  /* VFS only */
		open_no_follow = 0x01000000,  /* Ok for sqlite3_open_v2() */
		open_ex_res_code = 0x02000000,  /* Extended result codes */
		open_rw_create_ex_res_code = 0x00000004 | 0x00000002 | 0x02000000, /*Ok for sqlite3_open_v2(), combination of open_read_write, open_create, open_ex_res_code*/
	};
}

#endif //H_MLN_DB_DB_FLAGS_H