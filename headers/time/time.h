#pragma once
#ifndef H_MLN_DB_TIME_H
#define H_MLN_DB_TIME_H

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace mln {
	class time {
	public:
		struct date {
			int day, month, year, hour, minute, second;
		};

		[[nodiscard]] inline static constexpr uint64_t get_date_to_seconds_size() noexcept
		{
			//Format: [dd/MM hh:mm:ss]. Example: 21/05 01:12:33
			return 14;
		}
		[[nodiscard]] inline static constexpr uint64_t get_cd_to_seconds_size() noexcept
		{
			//Format: [m:ss]. Example: 3:11
			return 4;
		}

		static std::string get_current_date_time();
		static std::optional<uint64_t> convert_date_to_seconds_left(const std::string& date);
		static std::optional<uint64_t> convert_cd_to_seconds_left(const std::string& cooldown, const uint64_t negative_offset);
		static std::string convert_timestamp_to_mmss(const uint64_t timestamp);
		static uint64_t convert_date_to_UNIX(const time::date& date);
		static time::date convert_UNIX_to_date(const uint64_t unix_time);
		static uint64_t get_current_UNIX_time();

		static [[nodiscard]] std::chrono::seconds get_current_time_sec() noexcept;
		static [[nodiscard]] std::chrono::milliseconds get_current_time_milli() noexcept;
		static [[nodiscard]] std::chrono::microseconds get_current_time_micro() noexcept;
		static [[nodiscard]] std::chrono::nanoseconds get_current_time_nano() noexcept;
	};
}

#endif // H_MLN_DB_TIME_H