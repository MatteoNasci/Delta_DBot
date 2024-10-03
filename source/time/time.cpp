#include "time/time.h"

#include <corecrt.h>
#include <ctime>
#include <mutex>
#include <string>

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