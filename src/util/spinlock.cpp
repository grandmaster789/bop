#include "spinlock.h"
#include <thread>

namespace bop::util {
	/***** SpinLock *****/
	void Spinlock::lock() noexcept {
		while (true) {
			if (!m_Lock.exchange(true, std::memory_order_acquire))
				return;

			while (m_Lock.load(std::memory_order_relaxed))
				std::this_thread::yield();
		}
	}

	bool Spinlock::try_lock() noexcept {
		return !m_Lock.exchange(true, std::memory_order_acquire);
	}

	void Spinlock::unlock() noexcept {
		m_Lock.store(false, std::memory_order_release);
	}

	/***** RecursiveSpinlock *****/
	std::size_t RecursiveSpinLock::get_local_hash() noexcept {
		static std::atomic<std::size_t> s_shared_id = 1;
		static thread_local std::size_t t_hash      = s_shared_id.fetch_add(1);
		return t_hash;
	}

	void RecursiveSpinLock::lock() noexcept {
		std::size_t x    = get_local_hash();
		std::size_t sema = m_Semaphore.load(std::memory_order_relaxed);

		if (sema == x)
			++m_Count;
		else {
			while (true) {
				std::size_t value = 0;

				if (m_Semaphore.compare_exchange_strong(
					value, 
					x, 
					std::memory_order_acquire, 
					std::memory_order_relaxed
				)) {
					++m_Count;
					return;
				}

				while (m_Semaphore.load(std::memory_order_relaxed) != 0)
					std::this_thread::yield();
			}
		}
	}

	bool RecursiveSpinLock::try_lock() noexcept {
		std::size_t x         = get_local_hash();
		std::size_t sema = m_Semaphore.load(std::memory_order_relaxed);

		if (!sema) {
			std::size_t value = 0;

			if (m_Semaphore.compare_exchange_strong(
				value,
				x,
				std::memory_order_acquire,
				std::memory_order_relaxed
			)) {
				++m_Count;
				return true;
			}

			return false;
		}
		else if (sema == x) {
			++m_Count;
			return true;
		}

		return false;
	}

	void RecursiveSpinLock::unlock() noexcept {
		--m_Count;
		if (!m_Count)
			m_Semaphore.store(0, std::memory_order_release);
	}
}