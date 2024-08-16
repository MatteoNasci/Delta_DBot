#pragma once
#ifndef H_MLN_DB_ADD_EMOJI_H
#define H_MLN_DB_ADD_EMOJI_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class add_emoji final : public base_slashcommand {
    public:
        add_emoji(bot_delta* const delta);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) override;
    };
}

#endif //H_MLN_DB_ADD_EMOJI_H