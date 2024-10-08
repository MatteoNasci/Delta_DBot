#pragma once
#ifndef H_MLN_DB_DB_INIT_TYPE_FLAG_H
#define H_MLN_DB_DB_INIT_TYPE_FLAG_H

#include <type_traits>

namespace mln {
	enum class db_init_type_flag : unsigned int {
		none = 0,
		thinking = 1 << 0,
		cmd_data = 1 << 1,
		dump_channel = 1 << 2,
		all = 0xFFFFFFFF,
	};

    // Enable bitwise operations for enum class
    inline db_init_type_flag operator|(db_init_type_flag lhs, db_init_type_flag rhs) {
        return static_cast<db_init_type_flag>(
            static_cast<std::underlying_type_t<db_init_type_flag>>(lhs) |
            static_cast<std::underlying_type_t<db_init_type_flag>>(rhs)
            );
    }

    inline db_init_type_flag& operator|=(db_init_type_flag& lhs, db_init_type_flag rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    inline db_init_type_flag operator&(db_init_type_flag lhs, db_init_type_flag rhs) {
        return static_cast<db_init_type_flag>(
            static_cast<std::underlying_type_t<db_init_type_flag>>(lhs) &
            static_cast<std::underlying_type_t<db_init_type_flag>>(rhs)
            );
    }

    inline db_init_type_flag& operator&=(db_init_type_flag& lhs, db_init_type_flag rhs) {
        lhs = lhs & rhs;
        return lhs;
    }
}

#endif //H_MLN_DB_DB_INIT_TYPE_FLAG_H