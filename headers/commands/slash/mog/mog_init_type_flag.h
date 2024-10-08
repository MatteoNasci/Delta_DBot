#pragma once
#ifndef H_MLN_DB_MOG_INIT_TYPE_FLAG_H
#define H_MLN_DB_MOG_INIT_TYPE_FLAG_H

#include <type_traits>

namespace mln {
    namespace mog {
        enum class mog_init_type_flag : unsigned int {
            none = 0,
            thinking = 1 << 0,
            cmd_data = 1 << 1,
            all = 0xFFFFFFFF,
        };

        // Enable bitwise operations for enum class
        inline mog_init_type_flag operator|(mog_init_type_flag lhs, mog_init_type_flag rhs) {
            return static_cast<mog_init_type_flag>(
                static_cast<std::underlying_type_t<mog_init_type_flag>>(lhs) |
                static_cast<std::underlying_type_t<mog_init_type_flag>>(rhs)
                );
        }

        inline mog_init_type_flag& operator|=(mog_init_type_flag& lhs, mog_init_type_flag rhs) {
            lhs = lhs | rhs;
            return lhs;
        }

        inline mog_init_type_flag operator&(mog_init_type_flag lhs, mog_init_type_flag rhs) {
            return static_cast<mog_init_type_flag>(
                static_cast<std::underlying_type_t<mog_init_type_flag>>(lhs) &
                static_cast<std::underlying_type_t<mog_init_type_flag>>(rhs)
                );
        }

        inline mog_init_type_flag& operator&=(mog_init_type_flag& lhs, mog_init_type_flag rhs) {
            lhs = lhs & rhs;
            return lhs;
        }
    }
}

#endif //H_MLN_DB_MOG_INIT_TYPE_FLAG_H