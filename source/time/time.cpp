#include "time/time.h"
#include "utility/utility.h"

#include <corecrt.h>
#include <time.h>
#include <ctime>
#include <mutex>
#include <string>
#include <cstdint>
#include <format>
#include <exception>
#include <optional>
#include <chrono>

constexpr uint64_t mln::time::get_date_to_seconds_size()
{
    //Format: [dd/MM hh:mm:ss]. Example: 21/05 01:12:33
    return 14;
}

constexpr uint64_t mln::time::get_cd_to_seconds_size()
{
    //Format: [m:ss]. Example: 3:11
    return 4;
}

std::string mln::time::get_current_date_time()
{
    static constinit std::mutex s_localtime_mutex{};

    const time_t now = std::time(0);
    struct tm tstruct;
    {
        std::unique_lock lock{ s_localtime_mutex };
        struct tm* tstruct_ptr = std::localtime(&now);
        if (!tstruct_ptr) [[unlikely]] {
            return "Unknown time";
        }
        tstruct = *tstruct_ptr;
    }
    char buf[80];
    std::strftime(buf, sizeof(buf), "%d-%m-%Y %X", &tstruct);
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