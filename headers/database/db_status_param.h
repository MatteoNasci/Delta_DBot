#pragma once
#ifndef H_MLN_DB_DB_STATUS_PARAM_H
#define H_MLN_DB_DB_STATUS_PARAM_H

namespace mln {
	enum class db_status_param : int {
		/*This parameter is the current amount of memory checked out using sqlite3_malloc(), either directly or indirectly. The figure includes calls made to sqlite3_malloc() by the application and internal memory usage by the SQLite library. Auxiliary page-cache memory controlled by SQLITE_CONFIG_PAGECACHE is not included in this parameter. The amount returned is the sum of the allocation sizes as reported by the xSize method in sqlite3_mem_methods.*/
		memory_used = 0,
		/*This parameter returns the number of pages used out of the pagecache memory allocator that was configured using SQLITE_CONFIG_PAGECACHE. The value returned is in pages, not in bytes.*/
		page_cache_used = 1,
		/*This parameter returns the number of bytes of page cache allocation which could not be satisfied by the SQLITE_CONFIG_PAGECACHE buffer and where forced to overflow to sqlite3_malloc(). The returned value includes allocations that overflowed because they where too large (they were larger than the "sz" parameter to SQLITE_CONFIG_PAGECACHE) and allocations that overflowed because no space was left in the page cache.*/
		page_cache_overflow = 2,
		/* NOT USED */
		scratch_used = 3,
		/* NOT USED */
		scratch_overflow = 4,
		/*This parameter records the largest memory allocation request handed to sqlite3_malloc() or sqlite3_realloc() (or their internal equivalents). Only the value returned in the *pHighwater parameter to sqlite3_status() is of interest. The value written into the *pCurrent parameter is undefined.*/
		malloc_size = 5,
		/*The *pHighwater parameter records the deepest parser stack. The *pCurrent value is undefined. The *pHighwater value is only meaningful if SQLite is compiled with YYTRACKMAXSTACKDEPTH.*/
		parser_stack = 6,
		/*This parameter records the largest memory allocation request handed to the pagecache memory allocator. Only the value returned in the *pHighwater parameter to sqlite3_status() is of interest. The value written into the *pCurrent parameter is undefined.*/
		page_cache_size = 7,
		/* NOT USED */
		scratch_size = 8,
		/*This parameter records the number of separate memory allocations currently checked out.*/
		malloc_count = 9,
	};
}

#endif //H_MLN_DB_DB_STATUS_PARAM_H