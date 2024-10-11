#include "commands/slash/db/db_command_type.h"
#include "enum/flags.h"

const char* mln::get_cmd_type_text(const db_command_type type) noexcept {
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

    const auto casted_type = mln::flags::to_value(type);
    if (casted_type >= 0 && casted_type < s_type_to_text_map_size) {
        return s_type_to_text_map[casted_type];
    }

    return s_invalid_error_text;
}