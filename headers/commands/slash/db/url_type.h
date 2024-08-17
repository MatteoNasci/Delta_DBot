#pragma once
#ifndef H_MLN_DB_URL_TYPE_H
#define H_MLN_DB_URL_TYPE_H

namespace mln {
	enum class url_type : int {
		none = 0,
		file = 1,
		msg = 2,
	};
}

#endif //H_MLN_DB_URL_TYPE_H