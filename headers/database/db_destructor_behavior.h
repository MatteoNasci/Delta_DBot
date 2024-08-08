#pragma once
#ifndef H_MLN_DB_DB_DESTRUCTOR_BEHAVIOR_H
#define H_MLN_DB_DB_DESTRUCTOR_BEHAVIOR_H

namespace mln {
	enum class db_destructor_behavior {
		static_b = 0, /*If the destructor argument is SQLITE_STATIC, it means that the content pointer is constant and will never change. It does not need to be destroyed*/
		transient_b = 1, /*The SQLITE_TRANSIENT value means that the content will likely change in the near future and that SQLite should make its own private copy of the content before returning.*/
		free_b = 2, /*A destructor (free()) to dispose of the BLOB or string after SQLite has finished with it will be used. It is called to dispose of the BLOB or string even if the call to the bind API fails, except the destructor is not called if the third parameter (value ptr) is a NULL pointer or the fourth parameter (bytes) is negative.*/
		delete_b = 3, /*A destructor (delete) to dispose of the BLOB or string after SQLite has finished with it will be used. The pointer will be casted to unsigned char* to avoid undefined behavior from delete. It is called to dispose of the BLOB or string even if the call to the bind API fails, except the destructor is not called if the third parameter (value ptr) is a NULL pointer or the fourth parameter (bytes) is negative.*/
	};
}

#endif //H_MLN_DB_DB_DESTRUCTOR_BEHAVIOR_H