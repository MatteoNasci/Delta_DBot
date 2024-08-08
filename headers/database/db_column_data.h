#pragma once
#ifndef H_MLN_DB_DB_COLUMN_DATA_H
#define H_MLN_DB_DB_COLUMN_DATA_H

#include <variant>
#include <cstdint>

namespace mln {
	struct db_column_data_t {
		//I don't store db::fundamental_datatype here since I assume the caller knows the type of the information he's requesting and how to cast the variant to the appropriate type.
		//If the value held in the column is NULL, then the value set in the variant will be const short* = nullptr. (I use short to separate it from all the other types and avoid conflicts, afterall nullptr may be a valid db value for the other pointers)
		//const short* will only and only be used to hold NULL when the column item is NULL. The db values that can be NULL should always check if the variant hold a const short* value, and if it does it's always nullptr.
		//All pointers will have their lifetime linked to the statement used (or to new steps in the db execution process), so they should be copied if their values are needed to be saved and used later on.
		//const void* can hold either a blob item or a text16

		//int64_t == sqlite3_int64

		const char* name;
		const std::variant<const unsigned char*, const void*, int, int64_t, double, const short*> data;
		const int bytes;

		db_column_data_t() = delete;
		db_column_data_t(const char* name, const double data, const int bytes = sizeof(const double));
		db_column_data_t(const char* name, const int data, const int bytes = sizeof(const int));
		db_column_data_t(const char* name, const int64_t data, const int bytes = sizeof(const int64_t));
		db_column_data_t(const char* name, const void* data, const int bytes = sizeof(const void*));
		db_column_data_t(const char* name, const unsigned char* data, const int bytes = sizeof(const unsigned char*));
		//This data should always be set to nullptr!
		db_column_data_t(const char* name, const short* data = nullptr, const int bytes = sizeof(const short*));
	};
}

#endif