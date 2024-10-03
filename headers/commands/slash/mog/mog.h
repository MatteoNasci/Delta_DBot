#pragma once
#ifndef H_MLN_DB_MOG_MOG_H
#define H_MLN_DB_MOG_MOG_H

#include "commands/slash/base_slashcommand.h"

#include <dpp/coro/job.h>

#include <functional>
#include <optional>

namespace dpp {
    class cluster;
    struct slashcommand_t;
}

namespace mln {
    class database_handler;
	namespace mog {
		class mog final : public base_slashcommand {
        private:
            database_handler& database;
        public:
            mog(dpp::cluster& cluster, database_handler& database);
            dpp::job command(dpp::slashcommand_t event_data) const override final;

            std::optional<std::function<void()>> job(dpp::slashcommand_t event_data) const override final;
            bool use_job() const override final;
		};
	}
}

#endif //H_MLN_DB_MOG_MOG_H