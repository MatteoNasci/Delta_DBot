#pragma once
#ifndef H_MLN_DB_VERSION_H
#define H_MLN_DB_VERSION_H

#define MLN_DB_VERSION_MAJOR 0
#define MLN_DB_VERSION_MINOR 17
#define MLN_DB_VERSION_PATCH 1
#define MLN_DB_VERSION_TWEAK 1

#define MLN_DB_STRINGIFY(x) #x
#define MLN_DB_EXPAND_AND_STRINGIFY(x) MLN_DB_STRINGIFY(x)

#define MLN_DB_VERSION \
    MLN_DB_EXPAND_AND_STRINGIFY(MLN_DB_VERSION_MAJOR) "." \
    MLN_DB_EXPAND_AND_STRINGIFY(MLN_DB_VERSION_MINOR) "." \
    MLN_DB_EXPAND_AND_STRINGIFY(MLN_DB_VERSION_PATCH) "." \
    MLN_DB_EXPAND_AND_STRINGIFY(MLN_DB_VERSION_TWEAK)

namespace mln {
	static constexpr unsigned long long int get_version_major() {
		return MLN_DB_VERSION_MAJOR;
	}

	static constexpr unsigned long long int get_version_minor() {
		return MLN_DB_VERSION_MINOR;
	}

	static constexpr unsigned long long int get_version_patch() {
		return MLN_DB_VERSION_PATCH;
	}

	static constexpr unsigned long long int get_version_tweak() {
		return MLN_DB_VERSION_TWEAK;
	}

	static constexpr const char* get_version() {
		return MLN_DB_VERSION;
	}
}

#endif //H_MLN_DB_VERSION_H