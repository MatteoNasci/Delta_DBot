#pragma once
#ifndef H_MLN_DB_LOGS_H
#define H_MLN_DB_LOGS_H

#include <dpp/misc-enum.h>

#include <cstdio>
#include <memory>
#include <string>

namespace dpp {
	struct log_t;
}

namespace mln {
	class logs {
	private:
		struct file_closer_t {
			void operator()(FILE* f) const;
		};

		std::unique_ptr<FILE, file_closer_t> log_file;
	public:
		logs(const char* const file_name);
		logs() = delete;
		static void log_to_file_and_terminal(const dpp::loglevel severity, const std::string& msg);
		static void log_to_file(const dpp::loglevel severity, const std::string& msg);
		static void log_to_file(const dpp::log_t& log);
	};
}

#endif