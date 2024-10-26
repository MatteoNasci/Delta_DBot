#pragma once
#ifndef H_MLN_DB_MOG_COMMAND_TYPE_H
#define H_MLN_DB_MOG_COMMAND_TYPE_H

namespace mln {
    namespace mog {
		enum class mog_command_type {
			none = 0,
			single = 1,
			help,
			create,
			del,
			show,
			join,
			leave,
			leave_and_join,
			start,
			raw_cooldown,
			cooldown,
			show_cooldowns,
			generic,
			enum_count
		};

		extern const char* get_cmd_type_text(const mln::mog::mog_command_type type) noexcept;
    }
}

#endif //H_MLN_DB_MOG_COMMAND_TYPE_H