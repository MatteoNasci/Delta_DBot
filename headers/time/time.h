#pragma once
#ifndef H_MLN_DB_TIME_H
#define H_MLN_DB_TIME_H

#include <string>

namespace mln {
	class time {
	public:
		static std::string get_current_date_time();
	};
}

#endif // H_MLN_DB_TIME_H