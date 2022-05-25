#pragma once

// assisted reflection via manual indication of member pointers
// from https://www.youtube.com/watch?v=e5fdPoRVZcg
// https://github.com/dominicpoeschko/aglio

namespace bop::util {
	template <typename T>
	struct TypeDescriptor;

	/* usage: provide a specialization of TypeDescriptor for your object,
	        that inherits from MemberList with pointers to all members of interest

			i.e.
				struct Foo {
					int x;
					float y;
				};

				template <>
				struct TypeDescriptor<Foo>:
					MemberList<
						&Foo::x,
						&Foo::y
					>
				{};
	*/

	template <auto... MemberPointers>
	struct MemberList {
		template <typename T, typename Fn>
		constexpr static auto apply(Fn&& fn, T& object) {
			return fn(object.*MemberPointers...);
		}
	};

	template <typename T>
	concept Described = requires {
		TypeDescriptor<T>(); // must be able to instantiate a TypeDescriptor object
	};

	// reflection-based methods: ostream, serialize, deserialize
	template <typename t_ostream, Described T>
	t_ostream& operator << (t_ostream& os, const T& obj) {
		os << '(';
		
		TypeDescriptor<T>::apply(
			[&os](const auto&... members) {
				((os << " " << members), ...);
			}, 
			obj
		);

		os << ')';
	}

	template <typename B, typename...Ts> // ~~ buffer must have a .size(), .resize() and .data() so we can use memcpy
	bool serialize(B& buffer, const Ts&... members);

	namespace detail {
		// several overloads should be provided for basic types
		// NOTE this seems like it could use some work
		template <typename B, std::integral N>
		bool serialize_impl(B& buffer, const N& value) {
			static constexpr size_t k_NumBytes = sizeof(N);
			const auto              old_size   = buffer.size();
			
			buffer.resize(old_size + k_NumBytes);
			
			std::memcpy(
				buffer.data() + old_size, 
				std::addressof(value), 
				k_NumBytes
			);

			return true;
		}

		// finally an overload is provided for Described types in order to support nesting
		template <typename t_buffer, Described T>
		bool serialize_impl(t_buffer& buffer, const T& obj) {
			return TypeDescriptor<T>::apply(
				[&buffer](const auto&... members) {
					return serialize(buffer, members...);
				},
				obj
			);
		}
	}

	template <typename B, typename... Ts>
	bool serialize(B& buffer, const Ts&... values) {
		return (detail::serialize_impl((buffer, values) && ...));
	}
}