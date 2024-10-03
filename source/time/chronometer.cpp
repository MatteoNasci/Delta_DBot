#include "time/chronometer.h"

#include <chrono>

mln::chronometer::chronometer() noexcept : start{ std::chrono::system_clock::now() }, end{ start }
{
}

void mln::chronometer::stop() noexcept
{
	end = std::chrono::system_clock::now();
}
void mln::chronometer::restart() noexcept
{
	start = std::chrono::system_clock::now();
	end = start;
}
std::chrono::seconds mln::chronometer::get_time_sec() const noexcept
{
	return std::chrono::duration_cast<std::chrono::seconds>(end - start);
}
std::chrono::milliseconds mln::chronometer::get_time_milli() const noexcept
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}
std::chrono::microseconds mln::chronometer::get_time_micro() const noexcept
{
	return std::chrono::duration_cast<std::chrono::microseconds>(end - start);
}
std::chrono::nanoseconds mln::chronometer::get_time_nano() const noexcept
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
}
