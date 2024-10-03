#pragma once
#ifndef H_MLN_DB_CHRONOMETER_H
#define H_MLN_DB_CHRONOMETER_H

#include <chrono>

namespace mln {
	class chronometer {
	private:
		std::chrono::system_clock::time_point start;
		std::chrono::system_clock::time_point end;
	public:
		[[nodiscard]] chronometer() noexcept;

		[[nodiscard]] chronometer(const chronometer&) noexcept = default;

		[[nodiscard]] chronometer(chronometer&& rhs) noexcept = default;

		chronometer& operator=(const chronometer&) noexcept = default;

		chronometer& operator=(chronometer&& rhs) noexcept = default;

		void stop() noexcept;
		void restart() noexcept;
		[[nodiscard]] std::chrono::seconds get_time_sec() const noexcept;
		[[nodiscard]] std::chrono::milliseconds get_time_milli() const noexcept;
		[[nodiscard]] std::chrono::microseconds get_time_micro() const noexcept;
		[[nodiscard]] std::chrono::nanoseconds get_time_nano() const noexcept;
	};
}

#endif // H_MLN_DB_CHRONOMETER_H