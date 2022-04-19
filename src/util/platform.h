#pragma once

// simple platform detection
#define BOP_PLATFORM_WINDOWS 1
#define BOP_PLATFORM_LINUX   2

// may be oversimplified, but mostly true
#ifdef _WIN32
	#define BOP_PLATFORM BOP_PLATFORM_WINDOWS
#else
	#define BOP_PLATFORM BOP_PLATFORM_LINUX
#endif

#ifdef _DEBUG
	#define BOP_DEBUG
#endif

namespace bop {
	enum class e_Platform {
		windows = BOP_PLATFORM_WINDOWS,
		linux   = BOP_PLATFORM_LINUX,

		current = BOP_PLATFORM
	};

	static constexpr e_Platform k_Platform = e_Platform::current;

	// maybe add iostream formatting?
}