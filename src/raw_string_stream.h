#pragma once

#include <memory>  // unique_ptr
#include <cstddef> // std::byte
#include <string_view>
#include <string>

namespace bop {
	/*
	   string-like container for raw bytes
	   NOTE should compare against actual vector when this thing is completed
	   NOTE probably could make most of this constexpr
	*/
	class RawStringStream {
	public:
		using Buffer    = char[];
		using BufferPtr = std::unique_ptr<Buffer>;

		constexpr RawStringStream() noexcept;
		explicit  RawStringStream(size_t capacity);

		RawStringStream             (const RawStringStream&) = delete;
		RawStringStream& operator = (const RawStringStream&) = delete;
		RawStringStream             (RawStringStream&&) noexcept;
		RawStringStream& operator = (RawStringStream&&) noexcept;

		size_t      size()     const;
		size_t      capacity() const;
		const char* data()     const;
		const char* c_str()    const;

		void clear();
		void full_clear(); // includes zeroing the memory

		void resize (size_t capacity);
		void reserve(size_t capacity);
		void ensure (size_t num_bytes); // ensures that the indicated number can be pushed onto the buffer

		char& operator[](size_t idx);
		char  operator[](size_t idx) const;

		char& front();
		char  front() const;
		char& back();
		char  back() const;

		void swap(RawStringStream& rbs) noexcept;
		void swap(RawStringStream&& rbs) noexcept;

	//protected:
		RawStringStream& append(int8_t c);               // or a char, or a byte
		RawStringStream& append(size_t count, int8_t c); // repeat the indicated byte value
		RawStringStream& append(std::string_view sv);
		RawStringStream& append(const void* raw_data, size_t num_bytes); // mostly used to append C strings

		RawStringStream& operator << (bool b);
		
		RawStringStream& operator << (int8_t  v); // interpreted as a 'char'
		RawStringStream& operator << (int16_t v); // interpreted as a 'short'     (converted to C string)
		RawStringStream& operator << (int32_t v); // interpreted as an 'int'      (converted to C string)
		RawStringStream& operator << (int64_t v); // interpreted as a 'long long' (converted to C string)

		RawStringStream& operator << (uint8_t  uv); // interpreted as a 'char'
		RawStringStream& operator << (uint16_t uv); // interpreted as a 'unsigned short'     (converted to C string)
		RawStringStream& operator << (uint32_t uv); // interpreted as a 'unsigned int'       (converted to C string)
		RawStringStream& operator << (uint64_t uv); // interpreted as a 'unsigned long long' (converted to C string)

		RawStringStream& operator << (float v);
		RawStringStream& operator << (double d);

		RawStringStream& operator << (std::string_view sv);
		RawStringStream& operator << (const std::string& str);
		RawStringStream& operator << (const char* c_string);
		RawStringStream& operator << (const void* ptr); // interpreted as the pointers' value converted to C string
		RawStringStream& operator << (std::nullptr_t);  // interpreted as "0x0"

		size_t    m_Size;
		size_t    m_Capacity;
		BufferPtr m_Buffer;
	};
}

#include "raw_string_stream.inl"