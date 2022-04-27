#pragma once

#include "task_system.h"
#include "../util/typelist.h"
#include <type_traits>

namespace bop::task {
	auto TaskSystem::schedule(std::invocable auto&& fn) {
		std::function result = fn;

		return result;
	}
}