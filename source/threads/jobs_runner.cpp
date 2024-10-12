#include "threads/jobs_runner.h"

#include <functional>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <type_traits>

mln::jobs_runner::jobs_runner() : thread{}, cv{}, queue_lock{}, state_lock{}, stop_thread{ false }, immediate_stop_thread{ false }, is_running { false }, jobs_completed{ 0 }, jobs_added{ 0 }, jobs{}
{
	
}

mln::jobs_runner::~jobs_runner() {
	stop(false);
}

bool mln::jobs_runner::start()
{
	if (is_thread_running()) {
		return false;
	}

	set_running_and_stop_thread(true, false, false);
	thread = std::thread{ mln::jobs_runner::run_jobs, std::ref(*this)};
	
	return true;
}

bool mln::jobs_runner::stop(const bool complete_all_remaining_jobs)
{
	if (!is_thread_running()) {
		return false;
	}

	set_running_and_stop_thread(true, true, !complete_all_remaining_jobs);
	cv.notify_all();
	thread.join();
	set_is_running(false);

	return true;
}

bool mln::jobs_runner::is_thread_running() const
{
	std::shared_lock<std::shared_mutex> shared_lock{ state_lock };
	return is_running;
}

bool mln::jobs_runner::is_thread_stop_requested() const
{
	std::shared_lock<std::shared_mutex> shared_lock{ state_lock };
	return stop_thread;
}

bool mln::jobs_runner::is_thread_immediate_stop_requested() const
{
	std::shared_lock<std::shared_mutex> shared_lock{ state_lock };
	return immediate_stop_thread;
}

bool mln::jobs_runner::is_thread_stopping() const
{
	std::shared_lock<std::shared_mutex> shared_lock{ state_lock };
	return stop_thread && is_running;
}

unsigned long long mln::jobs_runner::get_total_jobs_completed() const
{
	return jobs_completed.load(std::memory_order_relaxed);
}

unsigned long long mln::jobs_runner::get_total_jobs_added() const
{
	return jobs_added.load(std::memory_order_relaxed);
}

unsigned long long mln::jobs_runner::get_jobs_in_queue() const
{
	const unsigned long long total = jobs_added.load(std::memory_order_relaxed);
	const unsigned long long completed = jobs_completed.load(std::memory_order_relaxed);
	return total - completed;
}

bool mln::jobs_runner::add_job(const std::function<void(void)>& job)
{	
	if (is_thread_stop_requested()) {
		return false;
	}

	{
		std::unique_lock<std::mutex> lock{ queue_lock };
		jobs.push(job);
	}
	cv.notify_one();
	jobs_added.fetch_add(1, std::memory_order_relaxed);

	return true;
}

bool mln::jobs_runner::add_job(std::function<void(void)>&& job)
{
	if (is_thread_stop_requested()) {
		return false;
	}

	{
		std::unique_lock<std::mutex> lock{ queue_lock };
		jobs.emplace(std::forward<std::function<void(void)>>(job));
	}
	cv.notify_one();
	jobs_added.fetch_add(1, std::memory_order_relaxed);

	return true;
}

void mln::jobs_runner::set_is_running(const bool value)
{
	if (value != is_thread_running()) {
		std::unique_lock<std::shared_mutex> unique_lock{ state_lock };
		is_running = value;
	}
}

void mln::jobs_runner::set_stop_thread(const bool value)
{
	if (value != is_thread_stop_requested()) {
		std::unique_lock<std::shared_mutex> unique_lock{ state_lock };
		stop_thread = value;
	}
}

void mln::jobs_runner::set_immediate_stop_thread(const bool value)
{
	if (value != is_thread_immediate_stop_requested()) {
		std::unique_lock<std::shared_mutex> unique_lock{ state_lock };
		immediate_stop_thread = value;
	}
}

void mln::jobs_runner::set_running_and_stop_thread(const bool running, const bool stop, const bool immediate_stop)
{
	bool current_running, current_stop, current_immediate_stop;
	{
		std::shared_lock<std::shared_mutex> shared_lock{ state_lock };
		current_running = this->is_running;
		current_stop = this->stop_thread;
		current_immediate_stop = this->immediate_stop_thread;
	}

	if (stop != current_stop || running != current_running || immediate_stop != current_immediate_stop) {
		std::unique_lock<std::shared_mutex> unique_lock{ state_lock };
		this->stop_thread = stop;
		this->is_running = running;
		this->immediate_stop_thread = immediate_stop;
	}
}

bool mln::jobs_runner::end_jobs_loop() const
{
	std::shared_lock<std::shared_mutex> shared_lock{ state_lock };
	return this->immediate_stop_thread || (this->stop_thread && get_jobs_in_queue() == 0);
}

void mln::jobs_runner::run_jobs(jobs_runner& runner)
{
	for (; !runner.end_jobs_loop();) {
		std::function<void()> job;
		{
			std::unique_lock<std::mutex> lock{ runner.queue_lock };
			runner.cv.wait(lock, [&runner] {
				return !runner.jobs.empty() || runner.is_thread_stop_requested();
				});

			if (runner.end_jobs_loop()) {
				break;
			}
			job = runner.jobs.front();
			runner.jobs.pop();
		}
		job();

		runner.jobs_completed.fetch_add(1, std::memory_order_relaxed);
	}

	return;
}
