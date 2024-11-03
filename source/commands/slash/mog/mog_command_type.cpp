#include "commands/slash/mog/mog_command_type.h"
#include "enum/flags.h"

const char* mln::mog::get_cmd_type_text(const mln::mog::mog_command_type type) noexcept
{
    static const char* s_invalid_error_text = "Unknown mog command type";
    static constexpr size_t s_type_to_text_map_size = 16;
    static constexpr const char* s_type_to_text_map[s_type_to_text_map_size]{
        "none",
        "single",
        "help",
        "create",
        "del",
        "show",
        "join",
        "leave",
        "leave_and_join",
        "start",
        "scheduled",
        "raw_cooldown",
        "cooldown",
        "show_cooldowns",
        "generic",
        "enum_count",
    };

    const auto casted_type = mln::flags::to_value(type);
    if (casted_type >= 0 && casted_type < s_type_to_text_map_size) {
        return s_type_to_text_map[casted_type];
    }

    return s_invalid_error_text;
}
