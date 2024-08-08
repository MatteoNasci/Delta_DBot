#pragma once
#ifndef H_MLN_DB_DB_TEXT_ENCODING_H
#define H_MLN_DB_DB_TEXT_ENCODING_H

namespace mln {	
	enum class db_text_encoding : unsigned char {
		utf8 = 1,    /* IMP: R-37514-35566 */
		utf16le = 2,    /* IMP: R-03371-37637 */
		utf16be = 3,    /* IMP: R-51971-34154 */
		utf16 = 4,    /* Use native byte order */
		any = 5,    /* Deprecated */
		utf16_aligned = 8,    /* sqlite3_create_collation only */
	};
}

#endif //H_MLN_DB_DB_TEXT_ENCODING_H