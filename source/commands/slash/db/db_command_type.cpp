#include "commands/slash/db/db_command_type.h"

#include <type_traits>

const char* mln::get_cmd_type_text(const db_command_type type) {
    static const char* s_invalid_error_text = "Unknown db command type";
    static constexpr size_t s_type_to_text_map_size = 17;
    static constexpr const char* s_type_to_text_map[s_type_to_text_map_size]{
        "none",
        "file",
        "text",
        "url",
        "single",
        "multiple",
        "user",
        "all",
        "help",
        "description",
        "update_dump_channel",
        "generic",
        "self",
        "guild",
        "policy",
        "nsfw",
        "enum_count",
    };

    const std::underlying_type<db_command_type>::type casted_type = static_cast<std::underlying_type<db_command_type>::type>(type);
    if (casted_type >= static_cast<std::underlying_type<db_command_type>::type>(db_command_type::none) && casted_type < s_type_to_text_map_size) {
        return s_type_to_text_map[casted_type];
    }

    return s_invalid_error_text;
}