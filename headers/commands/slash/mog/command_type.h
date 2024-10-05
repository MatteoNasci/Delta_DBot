#pragma once
#ifndef H_MLN_DB_MOG_COMMAND_TYPE_H
#define H_MLN_DB_MOG_COMMAND_TYPE_H

namespace mln {
    namespace mog {
		enum class command_type {
			none = 0,
			single = 1,
			help,
			create,
			del,
			show,
			join,
			leave,
			start,
			cooldown,
			track_cooldowns,
			enum_count
		};

		extern const char* get_cmd_type_text(const mln::mog::command_type type);
    }
}

#endif //H_MLN_DB_MOG_COMMAND_TYPE_H