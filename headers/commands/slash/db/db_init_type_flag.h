#pragma once
#ifndef H_MLN_DB_DB_INIT_TYPE_FLAG_H
#define H_MLN_DB_DB_INIT_TYPE_FLAG_H

namespace mln {
	enum class db_init_type_flag : unsigned int {
		none = 0,
		thinking = 1 << 0,
		cmd_data = 1 << 1,
		dump_channel = 1 << 2,
		all = 0xFFFFFFFF,
	};
}

#endif //H_MLN_DB_DB_INIT_TYPE_FLAG_H