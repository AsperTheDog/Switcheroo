#pragma once
#include <array>
#include <functional>
#include <vector>

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

template<usize Size>
using ByteArray = std::array<u8, Size>;

#define CONCAT2_FORCE_EXP(x, y) CONCAT2(x, y)
#define CONCAT2(x, y) x##y

#define PADDING(num_bytes) [[maybe_unused]] ByteArray<num_bytes> CONCAT2_FORCE_EXP(pad, __LINE__)
#define ZERO_PADDING(num_bytes) PADDING(num_bytes) {}

namespace swroo::utils
{
    enum DecryptResult : u8
    {
        SUCCESS = 0,
        FAILURE = 1,
        NOT_ENCRYPTED = 2,
    };

    [[nodiscard]] constexpr u32 MagicFromChars(const char p_First, const char p_Second, const char p_Third, const char p_Fourth)
    {
        return static_cast<u32>(p_First) | static_cast<u32>(p_Second) << 8 | static_cast<u32>(p_Third) << 16 | static_cast<u32>(p_Fourth) << 24;
    }

    [[nodiscard]] inline bool isZero(const u8* p_Data, const usize p_Size)
    {
        for (usize i = 0; i < p_Size; ++i)
            if (p_Data[i] != 0)
                return false;
        return true;
    }

    class CallListOnDestroy
    {
    public:
        using Callback = std::function<void()>;

        explicit CallListOnDestroy(const bool p_Reversed = false) : m_Reversed(p_Reversed) {}

        ~CallListOnDestroy()
        {
            if (m_Reversed)
            {
                for (auto it = m_Callbacks.rbegin(); it != m_Callbacks.rend(); ++it)
                    (*it)();
            }
            else
            {
                for (const Callback& callback : m_Callbacks)
                    callback();
            }
        }

        void addCallback(const Callback& callback) { m_Callbacks.push_back(callback); }

    private:
        std::vector<Callback> m_Callbacks;

        bool m_Reversed;
    };

    class CallOnDestroy
    {
    public:
        using Callback = std::function<void()>;

        explicit CallOnDestroy(Callback p_Callback) : m_Callback(std::move(p_Callback)) {}
        ~CallOnDestroy() { m_Callback(); }

    private:
        Callback m_Callback;
    };
}