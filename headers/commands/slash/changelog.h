#pragma once
#ifndef H_MLN_DB_CHANGELOG_H
#define H_MLN_DB_CHANGELOG_H

#include "commands/slash/base_slashcommand.h"

#include <vector>
#include <memory>
#include <string>

namespace mln {
    class changelog final : public base_slashcommand {
    private:
        std::shared_ptr<const std::vector<std::string>> text;
    public:
        changelog(bot_delta* const delta);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) override;
    };
}

#endif //H_MLN_DB_CHANGELOG_H