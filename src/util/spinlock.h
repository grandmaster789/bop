#pragma once

#include "cacheline.h"
#include <atomic>

namespace bop::util {
	class 
		alignas(hardware_constructive_interference_size) 
		Spinlock 
	{
	public:
		void lock()     noexcept;
		bool try_lock() noexcept; // returns true on succesful lock acquisition, immediately returns false on failure
		void unlock()   noexcept;

	private:
		std::atomic<bool> m_Lock = false;
	};

	class
		alignas(hardware_constructive_interference_size)
		RecursiveSpinLock
	{
	public:
		static std::size_t get_local_hash() noexcept;

		void lock()     noexcept;
		bool try_lock() noexcept;
		void unlock()   noexcept;

	private:
		std::atomic<std::size_t> m_Semaphore = 0;
		std::size_t              m_Count     = 0;
	};
}