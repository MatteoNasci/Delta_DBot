#pragma once
#ifndef H_MLN_DB_FLAGS_H
#define H_MLN_DB_FLAGS_H

#include <type_traits>

namespace mln {
	class flags final {
	public:
		template<typename T_enum_flag>
		[[nodiscard]] inline static constexpr bool has(const T_enum_flag& flags, const T_enum_flag& to_check) noexcept {
			return ((static_cast<std::underlying_type_t<T_enum_flag>>(flags) &
				static_cast<std::underlying_type_t<T_enum_flag>>(to_check)) ==
				static_cast<std::underlying_type_t<T_enum_flag>>(to_check));
		}
		template<typename T_enum_flag, typename... T_enum_flags>
		[[nodiscard]] inline static constexpr bool has(const T_enum_flag& flags, const T_enum_flag& to_check, const T_enum_flags&... flags_to_check) noexcept {
			return mln::flags::has(flags, mln::flags::add(to_check, flags_to_check...));
		}
		template<typename T_enum_flag>
		[[nodiscard]] inline static constexpr bool any(const T_enum_flag& flags, const T_enum_flag& to_check) noexcept {
			return ((static_cast<std::underlying_type_t<T_enum_flag>>(flags) &
				static_cast<std::underlying_type_t<T_enum_flag>>(to_check)) != 0);
		}
		template<typename T_enum_flag, typename... T_enum_flags>
		[[nodiscard]] inline static constexpr bool any(const T_enum_flag& flags, const T_enum_flag& to_check, const T_enum_flags&... flags_to_check) noexcept {
			return mln::flags::any(flags, mln::flags::add(to_check, flags_to_check...));
		}
		template<typename T_enum_flag>
		[[nodiscard]] inline static constexpr T_enum_flag add(const T_enum_flag& flags, const T_enum_flag& to_add) noexcept {
			return static_cast<T_enum_flag>((
				static_cast<std::underlying_type_t<T_enum_flag>>(flags) |
				static_cast<std::underlying_type_t<T_enum_flag>>(to_add)));
		}
		template<typename T_enum_flag, typename... T_enum_flags>
		[[nodiscard]] inline static constexpr T_enum_flag add(const T_enum_flag& flags, const T_enum_flag& to_add, const T_enum_flags&... flags_to_add) noexcept {
			return mln::flags::add(flags, mln::flags::add(to_add, flags_to_add...));
		}
		template<typename T_enum_flag>
		[[nodiscard]] inline static constexpr T_enum_flag sub(const T_enum_flag& flags, const T_enum_flag& to_sub) noexcept {
			return static_cast<T_enum_flag>((
				static_cast<std::underlying_type_t<T_enum_flag>>(flags) &
				(~static_cast<std::underlying_type_t<T_enum_flag>>(to_sub))));
		}
		template<typename T_enum_flag, typename... T_enum_flags>
		[[nodiscard]] inline static constexpr T_enum_flag sub(const T_enum_flag& flags, const T_enum_flag& to_sub, const T_enum_flags&... flags_to_sub) noexcept {
			return mln::flags::sub(flags, to_sub) & mln::flags::sub(flags, flags_to_sub...);
		}
		template<typename T_enum_flag>
		[[nodiscard]] inline static constexpr T_enum_flag com(const T_enum_flag& flags, const T_enum_flag& flag) noexcept {
			return static_cast<T_enum_flag>((
				static_cast<std::underlying_type_t<T_enum_flag>>(flags) &
				static_cast<std::underlying_type_t<T_enum_flag>>(flag)));
		}
		template<typename T_enum_flag, typename... T_enum_flags>
		[[nodiscard]] inline static constexpr T_enum_flag com(const T_enum_flag& flags, const T_enum_flag& flag, const T_enum_flags&... more_flags) noexcept {
			return mln::flags::com(flags, mln::flags::com(flag, more_flags...));
		}
		template<typename T_enum_flag>
		[[nodiscard]] inline static constexpr T_enum_flag toggle(const T_enum_flag& flags, const T_enum_flag& to_toggle) noexcept {
			return static_cast<T_enum_flag>((
				static_cast<std::underlying_type_t<T_enum_flag>>(flags) ^
				static_cast<std::underlying_type_t<T_enum_flag>>(to_toggle)));
		}
		template<typename T_enum_flag>
		[[nodiscard]] inline static constexpr T_enum_flag inv(const T_enum_flag& flags) noexcept {
			return static_cast<T_enum_flag>(
				~static_cast<std::underlying_type_t<T_enum_flag>>(flags));
		}
		template<typename T_enum_flag>
		[[nodiscard]] inline static constexpr std::underlying_type_t<T_enum_flag> to_value(const T_enum_flag& flags) noexcept {
			return static_cast<std::underlying_type_t<T_enum_flag>>(flags);
		}
		template<typename T_enum_flag>
		[[nodiscard]] inline static constexpr T_enum_flag to_enum(const std::underlying_type_t<T_enum_flag>& value) noexcept {
			return static_cast<T_enum_flag>(value);
		}
	};
}

#endif //H_MLN_DB_FLAGS_H