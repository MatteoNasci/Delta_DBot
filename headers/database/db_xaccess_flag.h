#pragma once
#ifndef H_MLN_DB_DB_XACCESS_FLAG_H
#define H_MLN_DB_DB_XACCESS_FLAG_H

namespace mln {
	enum class db_xaccess_flag {
		access_exist = 0,
		access_read_write = 1,   /* Used by PRAGMA temp_store_directory */
		access_read = 2,   /* Unused */
	};
}

#endif //H_MLN_DB_DB_XACCESS_FLAG_H