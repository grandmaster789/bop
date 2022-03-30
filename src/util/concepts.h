#pragma once

#include <type_traits>

namespace bop::util {
	// expressing std type traits as concepts
	// seems clear that we've got some combination explosion going on here -_-
	// (full list from https://en.cppreference.com/w/cpp/meta#Type_traits)

	// base types
	template <typename T> concept c_void                = std::is_void_v<T>;
	template <typename T> concept c_null_pointer        = std::is_null_pointer_v<T>;
	template <typename T> concept c_integral            = std::is_integral_v<T>;
	template <typename T> concept c_floating_point      = std::is_floating_point_v<T>;
	template <typename T> concept c_array               = std::is_array_v<T>;
	template <typename T> concept c_enum                = std::is_enum_v<T>;
	template <typename T> concept c_union               = std::is_union_v<T>;
	template <typename T> concept c_class               = std::is_class_v<T>;
	template <typename T> concept c_function            = std::is_function_v<T>;
	template <typename T> concept c_pointer             = std::is_pointer_v<T>;
	template <typename T> concept c_lvalue_ref          = std::is_lvalue_reference_v<T>;
	template <typename T> concept c_rvalue_ref          = std::is_rvalue_reference_v<T>;
	template <typename T> concept c_member_object_ptr   = std::is_member_object_pointer_v<T>;
	template <typename T> concept c_member_function_ptr = std::is_member_function_pointer_v<T>;

	// composite types
	template <typename T> concept c_fundamental    = std::is_fundamental_v<T>;    // arithmetic   || void           || nullptr
	template <typename T> concept c_arithmetic     = std::is_arithmetic_v<T>;     // integral     || floating_point
	template <typename T> concept c_scalar         = std::is_scalar_v<T>;         // arithmetic   || enum           || pointer || member_pointer || nullptr
	template <typename T> concept c_object         = std::is_object_v<T>;         // scalar       || array          || union   || class
	template <typename T> concept c_compound       = std::is_compound_v<T>;       // !fundamental
	template <typename T> concept c_reference      = std::is_reference_v<T>;      // lvalue_ref   || rvalue_ref
	template <typename T> concept c_member_pointer = std::is_member_pointer_v<T>; // T class::*

	// type properties
	template <typename T> concept c_const                         = std::is_const_v<T>;
	template <typename T> concept c_volatile                      = std::is_volatile_v<T>;
	template <typename T> concept c_trivial                       = std::is_trivial_v<T>;
	template <typename T> concept c_trivially_copyable            = std::is_trivially_copyable_v<T>;
	template <typename T> concept c_standard_layout               = std::is_standard_layout_v<T>;
	template <typename T> concept c_pod                           = std::is_standard_layout_v<T> && std::is_trivial_v<T>; // NOTE is_pod has been deprecated
	template <typename T> concept c_unique_object_representations = std::has_unique_object_representations_v<T>;
	template <typename T> concept c_empty                         = std::is_empty_v<T>;
	template <typename T> concept c_unsigned                      = std::is_unsigned_v<T>;
	template <typename T> concept c_bounded_array                 = std::is_bounded_array_v<T>;
	template <typename T> concept c_unbounded_array               = std::is_unbounded_array_v<T>;
	template <typename T> concept c_scoped_enum                   = std::is_scoped_enum_v<T>;

	// operations
	template <typename T> concept c_constructible                   = std::is_constructible_v<T>;
	template <typename T> concept c_default_constructible           = std::is_default_constructible_v<T>;
	template <typename T> concept c_copy_constructible              = std::is_copy_constructible_v<T>;
	template <typename T> concept c_move_constructible              = std::is_move_constructible_v<T>;
	template <typename T> concept c_trivially_constructible         = std::is_trivially_constructible_v<T>;
	template <typename T> concept c_trivially_default_constructible = std::is_trivially_default_constructible_v<T>;
	template <typename T> concept c_trivially_copy_constructible    = std::is_trivially_copy_constructible_v<T>;
	template <typename T> concept c_trivially_move_constructible    = std::is_trivially_move_constructible_v<T>;
	template <typename T> concept c_nothrow_constructible           = std::is_nothrow_constructible_v<T>;
	template <typename T> concept c_nothrow_default_constructible   = std::is_nothrow_default_constructible_v<T>;
	template <typename T> concept c_nothrow_copy_constructible      = std::is_nothrow_copy_constructible_v<T>;
	template <typename T> concept c_nothrow_move_constructible      = std::is_nothrow_move_constructible_v<T>;

	// NOTE specific assignment traits only have a single type parameter, while the overall trait has two. Seems weird to me.
	template <typename T, typename U> concept c_assignable                = std::is_assignable_v<T, U>;
	template <typename T>             concept c_copy_assignable           = std::is_copy_assignable_v<T>;
	template <typename T>             concept c_move_assignable           = std::is_move_assignable_v<T>;
	template <typename T, typename U> concept c_trivially_assignable      = std::is_trivially_assignable_v<T, U>;
	template <typename T>             concept c_trivially_copy_assignable = std::is_trivially_copy_assignable_v<T>;
	template <typename T>             concept c_trivially_move_assignable = std::is_trivially_move_assignable_v<T>;
	template <typename T, typename U> concept c_nothrow_assignable        = std::is_nothrow_assignable_v<T, U>;
	template <typename T>             concept c_nothrow_copy_assignable   = std::is_nothrow_copy_assignable_v<T>;
	template <typename T>             concept c_nothrow_move_assignable   = std::is_nothrow_move_assignable_v<T>;

	template <typename T>             concept c_virtual_destructor     = std::has_virtual_destructor_v<T>;
	template <typename T>             concept c_swappable              = std::is_swappable_v<T>;
	template <typename T>             concept c_nothrow_swappable      = std::is_nothrow_swappable_v<T>;
	template <typename T, typename U> concept c_swappable_with         = std::is_swappable_with_v<T, U>;
	template <typename T, typename U> concept c_nothrow_swappable_with = std::is_nothrow_swappable_with_v<T, U>;

	// properties
	template <typename T,                std::size_t t_Requirement> concept c_alignment_of = (std::alignment_of_v<T> == t_Requirement);
	template <typename T,                std::size_t t_Requirement> concept c_rank         = (std::rank_v<T> == t_Requirement);
	template <typename T, std::size_t N, std::size_t t_Requirement> concept c_extent       = (std::extent_v<T, N> == t_Requirement);

	// type relationships
	template <typename T, typename U> concept c_same                = std::is_same_v<T, U>;
	template <typename T, typename U> concept c_base_of             = std::is_base_of_v<T, U>; // T is base of U (U is the derived class)
	template <typename T, typename U> concept c_convertible         = std::is_convertible_v<T, U>;
	template <typename T, typename U> concept c_nothrow_convertible = std::is_nothrow_convertible_v<T, U>;

	// invocables
	template <                   typename t_Fn, typename...t_Args> concept c_invocable           = std::is_invocable_v<t_Fn, t_Args...>;
	template <typename t_Result, typename t_Fn, typename...t_Args> concept c_invocable_r         = std::is_invocable_r_v<t_Result, t_Fn, t_Args...>;
	template <                   typename t_Fn, typename...t_Args> concept c_nothrow_invocable   = std::is_nothrow_invocable_v<t_Fn, t_Args...>;
	template <typename t_Result, typename t_Fn, typename...t_Args> concept c_nothrow_invocable_r = std::is_nothrow_invocable_r_v<t_Result, t_Fn, t_Args...>;
} 
