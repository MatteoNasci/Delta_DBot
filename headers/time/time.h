#pragma once
#ifndef H_MLN_DB_TIME_H
#define H_MLN_DB_TIME_H

#include <string>
#include <cstdint>
#include <optional>
#include <corecrt.h>
#include <chrono>

namespace mln {
	class time {
	public:
		static constexpr uint64_t get_date_to_seconds_size();
		static constexpr uint64_t get_cd_to_seconds_size();

		static std::string get_current_date_time();
		static std::optional<uint64_t> convert_date_to_seconds_left(const std::string& date);
		static std::optional<uint64_t> convert_cd_to_seconds_left(const std::string& cooldown, const uint64_t negative_offset);

		static [[nodiscard]] std::chrono::seconds get_current_time_sec() noexcept;
		static [[nodiscard]] std::chrono::milliseconds get_current_time_milli() noexcept;
		static [[nodiscard]] std::chrono::microseconds get_current_time_micro() noexcept;
		static [[nodiscard]] std::chrono::nanoseconds get_current_time_nano() noexcept;
	};
}

#endif // H_MLN_DB_TIME_H