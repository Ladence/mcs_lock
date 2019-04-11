#pragma once
#include <atomic>

class mcs_lock {
public:
	class scoped_lock;

private:
	std::atomic<scoped_lock *> _tail;

public:
	mcs_lock(const mcs_lock&) = delete;
	mcs_lock& operator=(const mcs_lock&) = delete;

	mcs_lock() : _tail(nullptr) {}

	class scoped_lock {
		mcs_lock &_lock;
		volatile scoped_lock* _next;
		volatile bool _owned;

	public:
		scoped_lock(const scoped_lock&) = delete;
		scoped_lock& operator=(const scoped_lock&) = delete;

		scoped_lock(mcs_lock &lock) :
			 _lock(lock), _next(nullptr), _owned(false)
		{
			auto tail = _lock._tail.exchange(this);
			if (tail != nullptr)
			{
				tail->_next = this;
				while (!_owned)
				{
				}
			}
		}

		~scoped_lock()
		{
			scoped_lock *tail = this;
			if (!_lock._tail.compare_exchange_strong(tail, nullptr))
			{
				while (_next == nullptr)
				{
				}
				_next->_owned = true;
			}
		}
	};
};
