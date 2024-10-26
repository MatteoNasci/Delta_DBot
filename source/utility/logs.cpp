#include "time/time.h"
#include "utility/logs.h"

#include <dpp/dispatcher.h>
#include <dpp/misc-enum.h>

#include <cstdio>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

static constexpr std::string s_severity_map[] = { "Trace", "Debug", "Info", "Warning", "Error", "Critical" };

mln::logs::logs(const char* const file_name)
{
	log_file = std::unique_ptr<FILE, file_closer_t>{ std::freopen(file_name, "a", stderr) };
	if (!log_file) {
		throw std::exception("Failed to open log file!");
	}
}
mln::logs::logs(logs&& rhs) noexcept : log_file{ std::move(rhs.log_file) } {
	rhs.log_file = {};
}

mln::logs& mln::logs::operator=(logs&& rhs) noexcept
{
	if (this != &rhs) {
		log_file = std::move(rhs.log_file);
		rhs.log_file = {};
	}

    return *this;
}

void mln::logs::log_to_file_and_terminal(const dpp::loglevel severity, const std::string& msg)
{
	mln::logs::log_to_file(severity, msg);
	std::cout << "{ [" << mln::time::get_current_date_time() << " " << s_severity_map[severity] << "] " << msg << " }\n";
}

void mln::logs::log_to_file(const dpp::loglevel severity, const std::string& msg)
{
	std::clog << "{ [" << mln::time::get_current_date_time() << " " << s_severity_map[severity] << "] " << msg << " }\n";
}

void mln::logs::log_to_file(const dpp::log_t& log)
{
	mln::logs::log_to_file(log.severity, log.message);
}

void mln::logs::file_closer_t::operator()(FILE* f) const
{
	fclose(f);
}
