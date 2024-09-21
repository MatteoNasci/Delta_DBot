#pragma once
#ifndef H_MLN_DB_CHANGELOG_H
#define H_MLN_DB_CHANGELOG_H

#include "commands/slash/base_slashcommand.h"

#include <vector>
#include <string>
#include <memory>

namespace mln {
    class changelog final : public base_slashcommand {
    private:
    public:
        changelog(dpp::cluster& cluster);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) const override;
    };
}

#endif //H_MLN_DB_CHANGELOG_H