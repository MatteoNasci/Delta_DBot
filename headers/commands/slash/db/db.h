#pragma once
#ifndef H_MLN_DB_DB_H
#define H_MLN_DB_DB_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class database_handler;

    class db final : public base_slashcommand {
    private:
        database_handler& database;
    public:
        db(dpp::cluster& cluster, database_handler& database);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) const override;
    };
}

#endif //H_MLN_DB_DB_H