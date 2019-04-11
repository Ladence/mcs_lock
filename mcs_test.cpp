#include "mcs_lock.h"

#include <atomic>

class spin_lock
{
public:
	class scoped_lock;

private:
	std::atomic<scoped_lock *> _lock;

public:
	spin_lock(const spin_lock&) = delete;
	spin_lock& operator=(const spin_lock&) = delete;

	spin_lock() : _lock(nullptr) {}

	class scoped_lock {
		spin_lock &_lock;

	public:
		scoped_lock(const scoped_lock&) = delete;
		scoped_lock& operator=(const scoped_lock&) = delete;

		scoped_lock(spin_lock &lock) : _lock(lock) {
			scoped_lock *expect = nullptr;
			while (!_lock._lock.compare_exchange_weak(expect, this)) {
				expect = nullptr;
			}
		}
		~scoped_lock() {
			_lock._lock = nullptr;
		}
	};
};

template<class mutex, class lock_object>
void spinner(mutex *m, int *val, std::size_t n) {
	for (std::size_t i = 0; i < n; ++i) {
		lock_object lk(*m);
		(*val)++;
	}
}

#include <chrono>

template<typename Clock>
class timer {
	std::chrono::time_point<Clock> _t0;

public:
	timer() : _t0(Clock::now()) {}
	timer(const timer&) = delete;
	timer& operator=(const timer&) = delete;

	void reset() { _t0 = Clock::now(); }
	operator std::chrono::milliseconds() const {
		return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - _t0);
	}
};

#include <algorithm>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

template<class mutex, class lock_object>
void test(const std::string &name, std::size_t nthreads) {
	std::vector<std::thread> threads;
	mutex m;
	int i = 0;

	timer<std::chrono::high_resolution_clock> t;
	std::generate_n(std::back_inserter(threads), nthreads,
		[&m, &i, nthreads]() { return std::move(std::thread(&spinner<mutex, lock_object>, &m, &i, (1 << 28) / nthreads)); }); // 268435456 / n operations of increment
	std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
	std::cout << name << " " << nthreads << ":\t" << static_cast<std::chrono::milliseconds>(t).count() << "ms" << std::endl;
}

int main(void) {
	for (std::size_t i = 1; i <= std::thread::hardware_concurrency(); ++i) {
		test<spin_lock, spin_lock::scoped_lock>("spinlock", i);
	}
	// for (std::size_t i = 1; i <= std::thread::hardware_concurrency(); ++i) {
	// 	test<std::mutex, std::unique_lock<std::mutex> >("std::mutex", i);
	// }
	for (std::size_t i = 1; i <= std::thread::hardware_concurrency(); ++i) {
		test<mcs_lock, mcs_lock::scoped_lock>("mcs_lock", i);
	}
	return 0;
}
