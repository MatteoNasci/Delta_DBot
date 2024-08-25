#pragma once
#ifndef H_MLN_DB_URL_H
#define H_MLN_DB_URL_H

#include <cstdint>

namespace mln {
	struct msg_url_t {
		uint64_t guild_id;
		uint64_t channel_id;
		uint64_t message_id;
	};
	struct attachment_url_t {
		uint64_t guild_id;
		uint64_t channel_id;
	};
}

#endif //H_MLN_DB_URL_H