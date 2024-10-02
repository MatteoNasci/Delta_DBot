#pragma once
#ifndef H_MLN_DB_CACHE_H
#define H_MLN_DB_CACHE_H

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace mln {

	template<typename T>
	struct default_hash {
		std::size_t operator()(const T& value) const {
			return std::hash<T>{}(value);
		}
	};

	template<typename T_key, typename T_value, bool use_unique_over_shared_ptr = false, 
		size_t purge_limit = 1000, size_t purge_count = 200, double decay_factor = 0.5, 
		bool automatic_decay_on_resize = true, typename T_hash = default_hash<T_key>>
	class cache {
		static_assert(purge_limit > 0, "purge_limit must be greater than 0");
		static_assert(purge_count > 0, "purge_count must be greater than 0");
		static_assert(purge_limit >= purge_count, "purge_limit must be greater or equal to purge_count");
		static_assert(decay_factor >= 0.0, "decay_factor must be greater or equal to 0");
		static_assert(decay_factor <= 1.0, "decay_factor must be lower or equal to 1");

	private:
		using ptr_type = typename std::conditional<use_unique_over_shared_ptr, std::unique_ptr<T_value>, std::shared_ptr<T_value>>::type;
		using get_return = typename std::conditional<use_unique_over_shared_ptr, T_value, std::shared_ptr<const T_value>>::type;

		std::unordered_map<T_key, std::tuple<ptr_type, size_t>, T_hash> mapped_storage;
		std::multimap<size_t, T_key> mapped_frequencies;
		mutable std::shared_mutex storage_mutex;

	public:
		cache() : mapped_storage(), mapped_frequencies() {
			mapped_storage.reserve(purge_limit);
		}

		get_return add_element(const T_key& key, T_value&& value) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);

			auto it = mapped_storage.find(key);
			if (it != mapped_storage.end()) {
				update_element_locked(it, std::forward<T_value>(value));
				return return_correct_element_locked(it);
			}

			if (get_count_locked() >= purge_limit) {
				remove_least_retrieved_elements_locked(purge_count);
			}

			mapped_frequencies.emplace(0, key);

			auto emplaced = mapped_storage.emplace(key, std::make_tuple(make_pointer_locked(std::forward<T_value>(value)), 0));
			return return_correct_element_locked(emplaced.first);
		}

		get_return add_element(const T_key& key, const T_value& value) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);

			auto it = mapped_storage.find(key);
			if (it != mapped_storage.end()) {
				update_element_locked(it, T_value{value});
				return return_correct_element_locked(it);
			}

			if (get_count_locked() >= purge_limit) {
				remove_least_retrieved_elements_locked(purge_count);
			}

			mapped_frequencies.emplace(0, key);

			auto emplaced = mapped_storage.emplace(key, std::make_tuple(make_pointer_locked(T_value{value}), 0));
			return return_correct_element_locked(emplaced.first);
		}

		get_return add_element(const T_key& key, ptr_type&& value) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);

			auto it = mapped_storage.find(key);
			if (it != mapped_storage.end()) {
				update_element_locked(it, std::forward<ptr_type>(value));
				return return_correct_element_locked(it);
			}

			if (get_count_locked() >= purge_limit) {
				remove_least_retrieved_elements_locked(purge_count);
			}

			mapped_frequencies.emplace(0, key);

			return return_correct_element_locked(mapped_storage.emplace(key, std::make_tuple(std::forward<ptr_type>(value), 0)).first);
		}

		std::optional<get_return> update_element(const T_key& key, T_value&& value) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);

			auto it = mapped_storage.find(key);
			if (it != mapped_storage.end()) {
				update_element_locked(it, std::forward<T_value>(value));
				return return_correct_element_locked(it);
			}

			return std::nullopt;
		}
		std::optional<get_return> update_element(const T_key& key, const T_value& value) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);

			auto it = mapped_storage.find(key);
			if (it != mapped_storage.end()) {
				update_element_locked(it, T_value{value});
				return return_correct_element_locked(it);
			}

			return std::nullopt;
		}

		std::optional<get_return> update_element(const T_key& key, ptr_type&& value) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);

			auto it = mapped_storage.find(key);
			if (it != mapped_storage.end()) {
				update_element_locked(it, std::forward<ptr_type>(value));
				return return_correct_element_locked(it);
			}

			return std::nullopt;
		}

		bool remove_element(const T_key& key) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);

			auto it = mapped_storage.find(key);
			if (it == mapped_storage.end()) {
				return false;
			}

			const size_t frequency = std::get<1>(it->second);
			mapped_storage.erase(it);

			auto range = mapped_frequencies.equal_range(frequency);
			for (auto freq_it = range.first; freq_it != range.second; ++freq_it) {
				if (freq_it->second == key) {
					mapped_frequencies.erase(freq_it);
					break;
				}
			}

			return true;
		}

		std::optional<get_return> get_element(const T_key& key) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);

			auto it = mapped_storage.find(key);
			if (it == mapped_storage.end()) {
				return std::nullopt;
			}

			update_frequency_locked(it);

			return return_correct_element_locked(it);
		}

		bool is_element_present(const T_key& key) const {
			std::shared_lock<std::shared_mutex> lock(storage_mutex);
			return mapped_storage.find(key) != mapped_storage.end();
		}

		bool remove_least_retrieved_elements(size_t count_to_remove) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);
			return remove_least_retrieved_elements_locked(count_to_remove);
		}

		size_t get_count() const {
			std::shared_lock<std::shared_mutex> lock(storage_mutex);
			return get_count_locked();
		}

		void apply_frequency_decay() {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);
			apply_frequency_decay_locked();
		}

		void clear() {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);
			mapped_storage.clear();
			mapped_frequencies.clear();
		}
	private:
		auto return_correct_element_locked(std::unordered_map<T_key, std::tuple<ptr_type, size_t>>::iterator& it) {
			if constexpr (use_unique_over_shared_ptr) {
				return T_value{*(std::get<0>(it->second))};
			} else {
				return std::get<0>(it->second);
			}
		}
		auto make_pointer_locked(T_value&& value) {
			if constexpr (use_unique_over_shared_ptr) {
				return std::make_unique<T_value>(std::forward<T_value>(value));
			} else {
				return std::make_shared<T_value>(std::forward<T_value>(value));
			}
		}

		void update_element_locked(std::unordered_map<T_key, std::tuple<ptr_type, size_t>>::iterator& it, T_value&& value) {
			std::get<0>(it->second) = make_pointer_locked(std::forward<T_value>(value));
		}
		void update_element_locked(std::unordered_map<T_key, std::tuple<ptr_type, size_t>>::iterator& it, ptr_type&& value) {
			std::get<0>(it->second) = ptr_type(std::forward<ptr_type>(value));
		}
		void update_frequency_locked(std::unordered_map<T_key, std::tuple<ptr_type, size_t>>::iterator& it) {
			const size_t old_frequency = std::get<1>(it->second);
			const size_t new_frequency = old_frequency + 1;

			// Remove the old frequency entry and reinsert the updated frequency in one step
			auto range = mapped_frequencies.equal_range(old_frequency);
			auto freq_it = std::find_if(range.first, range.second, [&it](const std::pair<size_t, T_key>& pair) { return pair.second == it->first; });
			if (freq_it != mapped_frequencies.end()) {
				mapped_frequencies.erase(freq_it);
			}

			std::get<1>(it->second) = new_frequency;
			mapped_frequencies.emplace(new_frequency, it->first);
		}

		size_t get_count_locked() const {
			return mapped_storage.size();
		}

		bool remove_least_retrieved_elements_locked(size_t count_to_remove) {
			size_t removed = 0;
			const size_t max_to_remove = std::min(count_to_remove, mapped_frequencies.size());
			while (removed < max_to_remove) {
				auto it = mapped_frequencies.begin();
				mapped_storage.erase(it->second);
				mapped_frequencies.erase(it);
				++removed;
			}

			if constexpr (automatic_decay_on_resize) {
				apply_frequency_decay_locked();
			}

			return count_to_remove == max_to_remove;
		}

		void apply_frequency_decay_locked() {
			std::multimap<size_t, T_key> new_mapped_frequencies;

			for (const std::pair<size_t, T_key>& pair : mapped_frequencies) {
				const size_t new_frequency = static_cast<size_t>(pair.first * decay_factor);
				new_mapped_frequencies.emplace(new_frequency, pair.second);
				std::get<1>(mapped_storage[pair.second]) = new_frequency;
			}

			mapped_frequencies = std::move(new_mapped_frequencies);
		}
	};



	template<typename T_key, typename T_value_primitive, size_t purge_limit = 10000, size_t purge_count = 1000, double decay_factor = 0.5, bool automatic_decay_on_resize = false>
	class cache_primitive {
		static_assert(purge_limit > 0, "purge_limit must be greater than 0");
		static_assert(purge_count > 0, "purge_count must be greater than 0");
		static_assert(purge_limit >= purge_count, "purge_limit must be greater or equal to purge_count");
		static_assert(decay_factor >= 0.0, "decay_factor must be greater or equal to 0");
		static_assert(decay_factor <= 1.0, "decay_factor must be lower or equal to 1");

	private:
		std::unordered_map<T_key, std::tuple<T_value_primitive, size_t>> mapped_storage;
		std::multimap<size_t, T_key> mapped_frequencies;
		mutable std::shared_mutex storage_mutex;

	public:
		cache_primitive() : mapped_storage{}, mapped_frequencies{} {
			mapped_storage.reserve(purge_limit);
		}

		T_value_primitive add_element(const T_key& key, const T_value_primitive& value) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);

			auto it = mapped_storage.find(key);
			if (it != mapped_storage.end()) {
				update_element_locked(it, value);
				return T_value_primitive{value};
			}

			if (get_count_locked() >= purge_limit) {
				remove_least_retrieved_elements_locked(purge_count);
			}

			mapped_storage.emplace(key, std::make_tuple(value, 0));

			mapped_frequencies.emplace(0, key);

			return T_value_primitive{value};
		}

		std::optional<T_value_primitive> update_element(const T_key& key, const T_value_primitive& value) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);

			auto it = mapped_storage.find(key);
			if (it != mapped_storage.end()) {
				update_element_locked(it, value);
				return T_value_primitive{value};
			}

			return std::nullopt;
		}

		bool remove_element(const T_key& key) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);

			auto it = mapped_storage.find(key);
			if (it == mapped_storage.end()) {
				return false;
			}

			const size_t frequency = std::get<1>(it->second);
			mapped_storage.erase(it);

			auto range = mapped_frequencies.equal_range(frequency);
			for (auto freq_it = range.first; freq_it != range.second; ++freq_it) {
				if (freq_it->second == key) {
					mapped_frequencies.erase(freq_it);
					break;
				}
			}

			return true;
		}

		std::optional<T_value_primitive> get_element(const T_key& key) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);

			auto it = mapped_storage.find(key);
			if (it == mapped_storage.end()) {
				return std::nullopt;
			}

			update_frequency_locked(it);

			return T_value_primitive{std::get<0>(it->second)};
		}

		bool is_element_present(const T_key& key) const {
			std::shared_lock<std::shared_mutex> lock(storage_mutex);
			return mapped_storage.find(key) != mapped_storage.end();
		}

		bool remove_least_retrieved_elements(size_t count_to_remove) {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);
			return remove_least_retrieved_elements_locked(count_to_remove);
		}

		size_t get_count() const {
			std::shared_lock<std::shared_mutex> lock(storage_mutex);
			return get_count_locked();
		}

		void apply_frequency_decay() {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);
			apply_frequency_decay_locked();
		}

		void clear() {
			std::unique_lock<std::shared_mutex> lock(storage_mutex);
			mapped_storage.clear();
			mapped_frequencies.clear();
		}
	private:
		void update_element_locked(std::unordered_map<T_key, std::tuple<T_value_primitive, size_t>>::iterator& it, const T_value_primitive& value) {
			std::get<0>(it->second) = value;
			update_frequency_locked(it);
		}
		void update_frequency_locked(std::unordered_map<T_key, std::tuple<T_value_primitive, size_t>>::iterator& it) {
			const size_t old_frequency = std::get<1>(it->second);
			const size_t new_frequency = old_frequency + 1;

			// Remove the old frequency entry and reinsert the updated frequency in one step
			auto range = mapped_frequencies.equal_range(old_frequency);
			auto freq_it = std::find_if(range.first, range.second, [&it](const std::pair<size_t, T_key>& pair) { return pair.second == it->first; });
			if (freq_it != mapped_frequencies.end()) {
				mapped_frequencies.erase(freq_it);
			}

			std::get<1>(it->second) = new_frequency;
			mapped_frequencies.emplace(new_frequency, it->first);
		}

		size_t get_count_locked() const {
			return mapped_storage.size();
		}

		bool remove_least_retrieved_elements_locked(size_t count_to_remove) {
			size_t removed = 0;
			const size_t max_to_remove = std::min(count_to_remove, mapped_frequencies.size());
			while (removed < max_to_remove) {
				auto it = mapped_frequencies.begin();
				mapped_storage.erase(it->second);
				mapped_frequencies.erase(it);
				++removed;
			}

			if (automatic_decay_on_resize) {
				apply_frequency_decay_locked();
			}

			return count_to_remove == max_to_remove;
		}

		void apply_frequency_decay_locked() {
			std::multimap<size_t, T_key> new_mapped_frequencies;

			for (const std::pair<size_t, T_key>& pair : mapped_frequencies) {
				const size_t new_frequency = static_cast<size_t>(pair.first * decay_factor);
				new_mapped_frequencies.emplace(new_frequency, pair.second);
				std::get<1>(mapped_storage[pair.second]) = new_frequency;
			}

			mapped_frequencies = std::move(new_mapped_frequencies);
		}
	};
}

#endif //H_MLN_DB_CACHE_H