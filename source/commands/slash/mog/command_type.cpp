#include "commands/slash/mog/command_type.h"

#include <type_traits>

const char* mln::mog::get_cmd_type_text(const mln::mog::command_type type)
{
    static const char* s_invalid_error_text = "Unknown mog command type";
    static constexpr size_t s_type_to_text_map_size = 12;
    static constexpr const char* s_type_to_text_map[s_type_to_text_map_size]{
        "none",
        "single",
        "help",
        "create",
        "del",
        "show",
        "join",
        "leave",
        "start",
        "cooldown",
        "track_cooldowns",
        "enum_count",
    };

    const std::underlying_type<mln::mog::command_type>::type casted_type = static_cast<std::underlying_type<mln::mog::command_type>::type>(type);
    if (casted_type >= static_cast<std::underlying_type<mln::mog::command_type>::type>(mln::mog::command_type::none) && casted_type < s_type_to_text_map_size) {
        return s_type_to_text_map[casted_type];
    }

    return s_invalid_error_text;
}
