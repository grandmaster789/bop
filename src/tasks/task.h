#pragma once

namespace bop::tasks {
	template <typename t_Result = void>
	class Task {
	public:
		template <typename Fn>
		explicit Task(Fn work);



	private:
	};
}