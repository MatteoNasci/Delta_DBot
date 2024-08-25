#pragma once
#ifndef H_MLN_DB_DB_COMMAND_TYPE_H
#define H_MLN_DB_DB_COMMAND_TYPE_H

namespace mln {
	enum class db_command_type {
		none = 0,
		file = 1,
		text = 2,
		url = 3,
		single = 4,
		multiple = 5,
		user = 6, 
		all = 7,
		help = 8,
		description = 9,
		update_dump_channel = 10,
		generic = 11,
	};
}

#endif //H_MLN_DB_DB_COMMAND_TYPE_H