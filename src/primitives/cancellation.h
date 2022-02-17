#pragma once

namespace bop::primitives {
	class CancellationSource;
	class Cancellation {
	public:
		friend class CancellationSource;

		static Cancellation none();

		bool can_be_cancelled() const;
		bool is_requested() const;

	private:
		static Cancellation s_None;
	};
}