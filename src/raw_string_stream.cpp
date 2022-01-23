#include "raw_string_stream.h"
#include <cassert>
#include <algorithm>
#include <array>
#include <format>
#include <charconv>

namespace bop {
	RawStringStream::RawStringStream(size_t capacity) :
		m_Size(0),
		m_Capacity(capacity)
	{
		// NOTE using the _for_overwrite indicates we do *not* require the memory to be initialized at all
		m_Buffer = std::make_unique_for_overwrite<Buffer>(capacity);
	}

	RawStringStream::RawStringStream(RawStringStream&& s) noexcept :
		m_Size    (s.m_Size),
		m_Capacity(s.m_Capacity),
		m_Buffer  (std::move(s.m_Buffer))
	{
		s.m_Size = 0;
		s.m_Capacity = 0;
	}

	RawStringStream& RawStringStream::operator = (RawStringStream&& s) noexcept {
		if (&s != this) {
			m_Buffer     = std::move(s.m_Buffer);
			m_Size       = s.m_Size;
			m_Capacity   = s.m_Capacity;
			s.m_Size     = 0;
			s.m_Capacity = 0;
		}

		return *this;
	}

	size_t RawStringStream::size() const {
		return m_Size;
	}

	size_t RawStringStream::capacity() const {
		return m_Capacity;
	}

	const char* RawStringStream::data() const {
		return m_Buffer.get();
	}

	const char* RawStringStream::c_str() const {
		// so here we sneak a null terminator inside even though this is a const function...
		// don't do this at home kids

		auto* mutable_self = const_cast<RawStringStream*>(this);

		mutable_self->reserve(m_Size + 1);
		if (m_Buffer[m_Size] != '\0')
			m_Buffer[m_Size]  = '\0';

		return m_Buffer.get();
	}

	void RawStringStream::clear() {
		m_Size = 0;
	}

	void RawStringStream::full_clear() {
		std::fill_n(m_Buffer.get(), m_Size, '\0');
		m_Size = 0;
	}

	void RawStringStream::resize(size_t capacity) {
		// NOTE this is a situation where realloc might be appropriate

		if (capacity != m_Capacity) {
			auto new_buffer = std::make_unique_for_overwrite<Buffer>(capacity); // buffer will be uninitialized
			
			std::copy_n(m_Buffer.get(), m_Size, new_buffer.get());

			m_Buffer   = std::move(new_buffer);
			m_Capacity = capacity;

			if (m_Size > capacity)
				m_Size = capacity;
		}
	}

	void RawStringStream::reserve(size_t capacity) {
		// only does something when the requested capacity is actually larger

		if (capacity > m_Capacity) {
			auto new_buffer = std::make_unique_for_overwrite<Buffer>(capacity); // buffer will be uninitialized

			std::copy_n(m_Buffer.get(), m_Size, new_buffer.get());

			m_Buffer   = std::move(new_buffer);
			m_Capacity = capacity;

			if (m_Size > capacity)
				m_Size = capacity;
		}
	}

	void RawStringStream::ensure(size_t num_bytes) {
		if (m_Capacity < m_Size + num_bytes)
			reserve(m_Capacity * 2 + num_bytes);
	}

	char& RawStringStream::operator[](size_t idx) {
		return m_Buffer[idx];
	}

	char RawStringStream::operator[](size_t idx) const {
		return m_Buffer[idx];
	}

	char& RawStringStream::front() {
		return m_Buffer[0];
	}

	char RawStringStream::front() const {
		return m_Buffer[0];
	}

	char& RawStringStream::back() {
		return m_Buffer[m_Size - 1];
	}

	char RawStringStream::back() const {
		return m_Buffer[m_Size - 1];
	}

	void RawStringStream::swap(RawStringStream& rbs) noexcept {
		std::swap(rbs.m_Size, m_Size);
		std::swap(rbs.m_Capacity, m_Capacity);
		std::swap(rbs.m_Buffer, m_Buffer);
	}

	void RawStringStream::swap(RawStringStream&& rbs) noexcept {
		rbs.swap(*this);
	}

	RawStringStream& RawStringStream::append(int8_t c) {
		static_assert(sizeof(int8_t) == sizeof(char));

		ensure(1);

		m_Buffer[m_Size++] = static_cast<char>(c);

		return *this;
	}

	RawStringStream& RawStringStream::append(size_t count, int8_t c) {
		static_assert(sizeof(int8_t) == sizeof(char));

		ensure(count);

		std::fill_n(m_Buffer.get() + m_Size, count, c);

		m_Size += count;

		return *this;
	}

	RawStringStream& RawStringStream::append(std::string_view sv) {
		ensure(sv.size());

		std::copy_n(sv.data(), sv.size(), m_Buffer.get() + m_Size);

		m_Size += sv.size();

		return *this;
	}

	RawStringStream& RawStringStream::append(const void* raw_data, size_t num_bytes) {
		ensure(num_bytes);

		std::copy_n(
			static_cast<const char*>(raw_data), 
			num_bytes, 
			m_Buffer.get() + m_Size
		);

		m_Size += num_bytes;

		return *this;
	}

	RawStringStream& RawStringStream::operator << (bool b) {
		if (b)
			return append("true", 4); // NOTE we're not copying a null terminator
		else
			return append("false", 5);
	}

	RawStringStream& RawStringStream::operator << (int8_t v) {
		return append(v);
	}

	RawStringStream& RawStringStream::operator << (int16_t v) {
		std::array<char, 6> buffer; // biggest string result is -32'768

		char* buffer_start = buffer.data();
		char* buffer_end   = buffer.data() + buffer.size();

		auto [ptr, error_code] = std::to_chars(buffer_start, buffer_end, v);

		if (ptr != buffer_end)
			return append(buffer_start, ptr - buffer_start);

		return *this; // silent failure
	}

	RawStringStream& RawStringStream::operator << (int32_t v) {
		std::array<char, 11> buffer; // biggest string result is -2'147'483'648

		char* buffer_start = buffer.data();
		char* buffer_end   = buffer.data() + buffer.size();

		auto [ptr, error_code] = std::to_chars(buffer_start, buffer_end, v);

		if (ptr != buffer_end)
			return append(buffer_start, ptr - buffer_start);

		return *this;
	}

	RawStringStream& RawStringStream::operator << (int64_t v) {
		std::array<char, 20> buffer; // biggest string result is -9'223'372'036'854'775'808

		char* buffer_start = buffer.data();
		char* buffer_end   = buffer.data() + buffer.size();

		auto [ptr, error_code] = std::to_chars(buffer_start, buffer_end, v);

		if (ptr != buffer_end)
			return append(buffer_start, ptr - buffer_start);

		return *this; // silent failure
	}

	RawStringStream& RawStringStream::operator << (uint8_t uv) {
		return append(std::bit_cast<int8_t>(uv));
	}

	RawStringStream& RawStringStream::operator << (uint16_t uv) {
		std::array<char, 5> buffer; // biggest string result is 32'768

		char* buffer_start = buffer.data();
		char* buffer_end   = buffer.data() + buffer.size();

		auto [ptr, error_code] = std::to_chars(buffer_start, buffer_end, uv);

		if (ptr != buffer_end)
			return append(buffer_start, ptr - buffer_start);

		return *this; // silent failure
	}

	RawStringStream& RawStringStream::operator << (uint32_t uv) {
		std::array<char, 10> buffer; // biggest string result is 2'147'483'647

		char* buffer_start = buffer.data();
		char* buffer_end   = buffer.data() + buffer.size();

		auto [ptr, error_code] = std::to_chars(buffer_start, buffer_end, uv);

		if (ptr != buffer_end)
			return append(buffer_start, ptr - buffer_start);

		return *this; // silent failure
	}

	RawStringStream& RawStringStream::operator << (uint64_t uv) {
		std::array<char, 19> buffer; // biggest string result is 9'223'372'036'854'775'807

		char* buffer_start = buffer.data();
		char* buffer_end   = buffer.data() + buffer.size();

		auto [ptr, error_code] = std::to_chars(buffer_start, buffer_end, uv);

		if (ptr != buffer_end)
			return append(buffer_start, ptr - buffer_start);

		return *this; // silent failure
	}

	RawStringStream& RawStringStream::operator << (float f) {
		std::array<char, 10> buffer;

		const auto result = std::format_to_n(
			buffer.data(), 
			buffer.size(), 
			"{}", 
			f
		);

		return append(buffer.data(), result.size);
	}

	RawStringStream& RawStringStream::operator << (double d) {
		std::array<char, 10> buffer;

		const auto result = std::format_to_n(
			buffer.data(),
			buffer.size(),
			"{}",
			d
		);

		return append(buffer.data(), result.size);
	}

	RawStringStream& RawStringStream::operator << (std::string_view sv) {
		return append(sv);
	}

	RawStringStream& RawStringStream::operator << (const std::string& s) {
		return append(std::string_view(s));
	}

	RawStringStream& RawStringStream::operator << (const char* c_string) {
		return append(c_string, std::strlen(c_string));
	}

	RawStringStream& RawStringStream::operator << (const void* ptr) {
		std::array<char, 12> buffer; // 0x123456789A

		const auto result = std::format_to_n(
			buffer.data(), 
			buffer.size(), 
			"{}", 
			ptr
		);

		return append(buffer.data(), result.size);
	}

	RawStringStream& RawStringStream::operator << (std::nullptr_t) {
		return append("0x0", 3);
	}
}