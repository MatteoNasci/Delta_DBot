#pragma once
#ifndef H_MLN_DB_REPLY_LOG_DATA_H
#define H_MLN_DB_REPLY_LOG_DATA_H

namespace dpp {
	class cluster;
	struct interaction_create_t;
}

namespace mln {
	struct reply_log_data_t {
		const dpp::interaction_create_t* const event_data;
		const dpp::cluster* const cluster;
		const bool is_first_response;

		reply_log_data_t(const dpp::interaction_create_t* const event_data, const dpp::cluster* const cluster, const bool is_first_response);
	};
}

#endif // H_MLN_DB_REPLY_LOG_DATA_H