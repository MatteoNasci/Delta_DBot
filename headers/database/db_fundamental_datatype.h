#pragma once
#ifndef H_MLN_DB_DB_FUNDAMENTAL_DATATYPE_H
#define H_MLN_DB_DB_FUNDAMENTAL_DATATYPE_H

namespace mln {
	enum class db_fundamental_datatype {
		integer_t = 1, /*64-bit signed integer*/
		float_t = 2, /*64-bit IEEE floating point number*/
		text_t = 3, /*string*/
		blob_t = 4, /*BLOB*/
		null_t = 5, /*NULL*/
	};
}

#endif //H_MLN_DB_DB_FUNDAMENTAL_DATATYPE_H