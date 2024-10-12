#pragma once
#ifndef H_MLN_DB_JOBS_RUNNER_H
#define H_MLN_DB_JOBS_RUNNER_H

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>

namespace mln {
	class jobs_runner {
	private:
		std::thread thread;

		std::condition_variable cv;
		mutable std::mutex queue_lock;
		mutable std::shared_mutex state_lock;

		bool stop_thread;
		bool immediate_stop_thread;
		bool is_running;

		std::atomic_ullong jobs_completed;
		std::atomic_ullong jobs_added;

		std::queue<std::function<void()>> jobs;
	public:
		jobs_runner();
		~jobs_runner();
		jobs_runner(const jobs_runner& rhs) = delete;
		jobs_runner(jobs_runner&& rhs) = delete;
		jobs_runner& operator=(const jobs_runner& rhs) = delete;
		jobs_runner& operator=(jobs_runner&& rhs) = delete;

		bool start();
		bool stop(const bool complete_all_remaining_jobs);

		bool is_thread_running() const;
		bool is_thread_stop_requested() const;
		bool is_thread_immediate_stop_requested() const;
		bool is_thread_stopping() const;
		unsigned long long get_total_jobs_completed() const;
		unsigned long long get_total_jobs_added() const;
		unsigned long long get_jobs_in_queue() const;

		[[nodiscard]] bool add_job(const std::function<void()>& job);
		[[nodiscard]] bool add_job(std::function<void()>&& job);
	private:

		void set_is_running(const bool value);
		void set_stop_thread(const bool value);
		void set_immediate_stop_thread(const bool value);
		void set_running_and_stop_thread(const bool running, const bool stop, const bool immediate_stop);

		bool end_jobs_loop() const;

	private:
		static void run_jobs(jobs_runner& runner);
	};
}

#endif // H_MLN_DB_JOBS_RUNNER_H