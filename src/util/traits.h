#pragma once

#include <memory_resource>
#include <type_traits>

namespace bop::util {
	template <typename>
	struct is_pmr_vector:
		std::false_type 
	{};


	template <typename T>
	struct is_pmr_vector<std::pmr::vector<T>>:
		std::true_type
	{};
}