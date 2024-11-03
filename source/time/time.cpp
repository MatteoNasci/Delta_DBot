#include "time/time.h"
#include "utility/utility.h"

#include <chrono>
#include <corecrt.h>
#include <cstdint>
#include <exception>
#include <format>
#include <mutex>
#include <optional>
#include <string>
#include <time.h>

static constinit std::mutex s_time_mutex{};

std::string mln::time::get_current_date_time()
{
    const time_t now = std::time(0);
    std::tm time_info;
    {
        std::unique_lock lock{ s_time_mutex };
        std::tm* const tstruct_ptr = std::localtime(&now);
        if (!tstruct_ptr) [[unlikely]] {
            return "Unknown time";
        }
        time_info = *tstruct_ptr;
    }
    char buf[80];
    std::strftime(buf, sizeof(buf), "%d-%m-%Y %X", &time_info);
    return std::string{ buf };
}

std::optional<uint64_t> mln::time::convert_date_to_seconds_left(const std::string& date)
{
    if (date.size() != mln::time::get_date_to_seconds_size()) {
        throw std::exception(std::format("Date converter accepts only strings of [{}] size!", std::to_string(mln::time::get_date_to_seconds_size())).c_str());
    }
    throw std::exception("Not implemented");
}

std::optional<uint64_t> mln::time::convert_cd_to_seconds_left(const std::string& cooldown, const uint64_t negative_offset)
{
    if (cooldown.size() != mln::time::get_cd_to_seconds_size()) {
        throw std::exception(std::format("Cooldown converter accepts only strings of [{}] size!", std::to_string(mln::time::get_cd_to_seconds_size())).c_str());
    }

    if (!mln::utility::is_digit(cooldown[0])) {
        return std::nullopt;
    }

    uint64_t seconds = static_cast<uint64_t>(mln::utility::to_digit(cooldown[0])) * 60;

    if (!mln::utility::is_digit(cooldown[2]) || !mln::utility::is_digit(cooldown[3])) {
        return std::nullopt;
    }

    seconds += (static_cast<uint64_t>(mln::utility::to_digit(cooldown[2])) * 10) + static_cast<uint64_t>(mln::utility::to_digit(cooldown[3])) - negative_offset;

    return mln::time::get_current_time_sec().count() + seconds;
}

std::string mln::time::convert_timestamp_to_mmss(const uint64_t timestamp)
{
    static constexpr uint64_t s_seconds_in_day = 86400;
    static constexpr uint64_t s_seconds_in_hour = 3600;
    static constexpr uint64_t s_seconds_in_minute = 60;

    const uint64_t start_day_timestamp = timestamp % s_seconds_in_day;

    const uint64_t start_hour_timestamp = start_day_timestamp % s_seconds_in_hour;

    const uint64_t minutes = start_hour_timestamp / s_seconds_in_minute;
    const uint64_t seconds = start_hour_timestamp % s_seconds_in_minute;

    static constexpr uint64_t s_zeroed_limit = 10;
    const bool are_minutes_zeroed = minutes < s_zeroed_limit;
    const bool are_seconds_zeroed = seconds < s_zeroed_limit;

    if (are_minutes_zeroed) {
        return are_seconds_zeroed ? std::format("0{}:0{}", minutes, seconds) : std::format("0{}:{}", minutes, seconds);
    }
    else {
        return are_seconds_zeroed ? std::format("{}:0{}", minutes, seconds) : std::format("{}:{}", minutes, seconds);
    }
}

mln::time::date mln::time::convert_UNIX_to_date(const uint64_t unix_time)
{
    const std::time_t time = static_cast<std::time_t>(unix_time);

    std::tm time_info;
    {
        std::unique_lock lock{ s_time_mutex };
        std::tm* const tstruct_ptr = std::gmtime(&time);
        if (!tstruct_ptr) [[unlikely]] {
            return mln::time::date{-1, -1, -1, -1, -1, -1};
        }
        time_info = *tstruct_ptr;
    }

    // Extract individual components
    mln::time::date result{};
    result.year = time_info.tm_year + 1900;  // tm_year is years since 1900
    result.month = time_info.tm_mon + 1;     // tm_mon is months since January (0-11)
    result.day = time_info.tm_mday;
    result.hour = time_info.tm_hour;
    result.minute = time_info.tm_min;
    result.second = time_info.tm_sec;

    return result;
}

uint64_t mln::time::get_current_UNIX_time()
{
    return mln::time::get_current_time_sec().count();
}

uint64_t mln::time::convert_date_to_UNIX(const time::date& date)
{
    std::tm timeinfo = {};
    timeinfo.tm_mday = date.day;
    timeinfo.tm_mon = date.month - 1;  // tm_mon is 0-based
    timeinfo.tm_year = date.year - 1900;  // tm_year is years since 1900
    timeinfo.tm_hour = date.hour;
    timeinfo.tm_min = date.minute;
    timeinfo.tm_sec = date.second;

    // Convert to Unix timestamp (seconds since 1970-01-01)
    return std::mktime(&timeinfo);
}

std::chrono::seconds mln::time::get_current_time_sec() noexcept
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
}
std::chrono::milliseconds mln::time::get_current_time_milli() noexcept
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}
std::chrono::microseconds mln::time::get_current_time_micro() noexcept
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
}
std::chrono::nanoseconds mln::time::get_current_time_nano() noexcept
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch());
}