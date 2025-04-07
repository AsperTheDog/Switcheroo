#pragma once
#include <array>

typedef std::uint8_t u8;
typedef std::int8_t i8;
typedef std::uint16_t u16;
typedef std::int16_t i16;
typedef std::uint32_t u32;
typedef std::int32_t i32;
typedef std::uint64_t u64;
typedef std::int64_t i64;

typedef float f32;
typedef double f64;

typedef std::size_t usize;
typedef std::uint8_t byte;

template<u32 Size>
using ByteArray = ByteArray<Size>;

[[nodiscard]] constexpr u32 MagicFromChars(const char a, const char b, const char c, const char d)
{
    return static_cast<u32>(a) | static_cast<u32>(b) << 8 | static_cast<u32>(c) << 16 | static_cast<u32>(d) << 24;
}

#define CONCAT2(x, y) DO_CONCAT2(x, y)
#define DO_CONCAT2(x, y) x##y

#define INSERT_PADDING_BYTES(num_bytes) [[maybe_unused]] ByteArray<num_bytes> CONCAT2(pad, __LINE__) {}
#define INSERT_PADDING_WORDS(num_words) [[maybe_unused]] std::array<u32, num_words> CONCAT2(pad, __LINE__) {}

#define INSERT_PADDING_BYTES_NOINIT(num_bytes) [[maybe_unused]] ByteArray<num_bytes> CONCAT2(pad, __LINE__)
#define INSERT_PADDING_WORDS_NOINIT(num_words) [[maybe_unused]] std::array<u32, num_words> CONCAT2(pad, __LINE__)