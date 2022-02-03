#pragma once

#include <functional>

namespace bop {
	class Pool {
	public:
		Pool();
		~Pool();

		Pool(
			std::function<void*()>&&     create_callback_fn,
			std::function<void(void*)>&& destroy_callback_fn,
			size_t max_capacity
		);

		// not 100% sure I even want copy/move to be available here
		Pool             (const Pool& p);
		Pool& operator = (const Pool& p);
		Pool             (Pool&& p) noexcept;
		Pool& operator = (Pool&& p) noexcept;

		void   push(void* element);
		void*  pop();
		size_t size() const;
		void   clear();

	private:
		uint32_t* m_Handle;
	};
}