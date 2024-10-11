#include "database/db_saved_stmt_state.h"
#include "enum/flags.h"

const char* mln::get_saved_stmt_state_text(const db_saved_stmt_state type) noexcept {
    static const char* s_invalid_error_text = "Unknown db saved stmt state";
    static constexpr size_t s_type_to_text_map_size = 4;
    static constexpr const char* s_type_to_text_map[s_type_to_text_map_size]{
        "none",
        "stmt_initialized",
        "params_initialized",
        "initialized"
    };

    const auto casted_type = mln::flags::to_value(type);
    if (casted_type >= 0 && casted_type < s_type_to_text_map_size) {
        return s_type_to_text_map[casted_type];
    }

    return s_invalid_error_text;
}