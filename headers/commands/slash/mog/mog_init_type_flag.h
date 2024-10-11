#pragma once
#ifndef H_MLN_DB_MOG_INIT_TYPE_FLAG_H
#define H_MLN_DB_MOG_INIT_TYPE_FLAG_H

namespace mln {
    namespace mog {
        enum class mog_init_type_flag : unsigned int {
            none = 0,
            thinking = 1 << 0,
            cmd_data = 1 << 1,
            all = 0xFFFFFFFF,
        };
    }
}

#endif //H_MLN_DB_MOG_INIT_TYPE_FLAG_H