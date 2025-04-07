#pragma once
#include "../util/common.hpp"

#include <filesystem>
#include <fstream>
#include <span>

namespace swroo {
    class MainFileReader
    {
    public:
        explicit MainFileReader(const std::filesystem::path& p_File);

        template<typename T>
        u32 read(T& p_Value, usize p_Offset = UINT64_MAX);

        template<typename T>
        u32 readBytes(T* p_Buffer, usize p_Size, usize p_Offset = UINT64_MAX);

        template<typename T>
        u32 readSpan(std::span<T> p_Vector, usize p_Size, usize p_Offset = UINT64_MAX);

        [[nodiscard]] usize getFileSize() const;
        [[nodiscard]] usize getCurrentPosition();
        void setCurrentPosition(u32 p_Position);

        void release();
        void addRef();

    private:
        std::ifstream m_File;
        std::filesystem::path m_FilePath;
        usize m_FileSize = 0;

        u32 m_InUseCount = 1;
    };

    class SubFileReader
    {
    public:
        explicit SubFileReader(MainFileReader& p_MainFile, usize p_Offset, usize p_Size);
        ~SubFileReader();

        template<typename T>
        u32 read(T& p_Value, usize p_Offset = UINT64_MAX);

        template<typename T>
        u32 readBytes(T* p_Buffer, usize p_Size, usize p_Offset = UINT64_MAX);

        template<typename T>
        u32 readSpan(std::span<T> p_Vector, usize p_Size, usize p_Offset = UINT64_MAX);

        [[nodiscard]] usize getFileSize() const;
        [[nodiscard]] usize getCurrentPosition() const;
        void setCurrentPosition(u32 p_Position);

        void release();

    private:
        MainFileReader& m_MainFile;
        usize m_Offset = 0;
        usize m_Size = 0;

        usize m_InternalOffset = 0;

        bool m_Released = false;
    };

    inline MainFileReader::MainFileReader(const std::filesystem::path& p_File)
    {
        m_FilePath = p_File;
        m_File.open(m_FilePath, std::ios::binary | std::ios::ate);
        if (!m_File.is_open())
        {
            throw std::runtime_error("Failed to open file: " + m_FilePath.string());
        }
        m_FileSize = static_cast<usize>(getCurrentPosition());
        m_File.seekg(0, std::ios::beg);
    }

    template <typename T>
    u32 MainFileReader::read(T& p_Value, const usize p_Offset)
    {
        if (p_Offset != UINT64_MAX)
        {
            m_File.seekg(p_Offset);
        }
        m_File.read(reinterpret_cast<char*>(&p_Value), sizeof(T));
        if (m_File.fail())
        {
            throw std::runtime_error("Failed to read from file: " + m_FilePath.string());
        }
        return static_cast<u32>(m_File.gcount());
    }

    template <typename T>
    u32 MainFileReader::readBytes(T* p_Buffer, usize p_Size, usize p_Offset)
    {
        if (p_Offset != UINT64_MAX)
        {
            m_File.seekg(p_Offset);
        }
        m_File.read(reinterpret_cast<char*>(p_Buffer), p_Size);
        if (m_File.fail())
        {
            throw std::runtime_error("Failed to read from file: " + m_FilePath.string());
        }
        return static_cast<u32>(m_File.gcount());
    }

    template <typename T>
    u32 MainFileReader::readSpan(std::span<T> p_Vector, const usize p_Size, const usize p_Offset)
    {
        if (p_Offset != UINT64_MAX)
        {
            m_File.seekg(p_Offset);
        }
        if (p_Vector.size() < p_Size)
        {
            throw std::runtime_error("Buffer size is smaller than requested size");
        }
        m_File.read(reinterpret_cast<char*>(p_Vector.data()), p_Size * sizeof(T));
        if (m_File.fail())
        {
            throw std::runtime_error("Failed to read from file: " + m_FilePath.string());
        }
        return static_cast<u32>(m_File.gcount());
    }

    inline usize MainFileReader::getFileSize() const
    {
        return m_FileSize;
    }

    inline usize MainFileReader::getCurrentPosition()
    {
        return m_File.tellg();
    }

    inline void MainFileReader::setCurrentPosition(const u32 p_Position)
    {
        m_File.seekg(p_Position);
        if (m_File.fail())
        {
            throw std::runtime_error("Failed to set file position: " + m_FilePath.string());
        }
    }

    inline void MainFileReader::release()
    {
        if (--m_InUseCount == 0)
        {
            m_File.close();
        }
    }

    inline void MainFileReader::addRef()
    {
        if (m_InUseCount == 0)
            m_File.open(m_FilePath, std::ios::binary);

        ++m_InUseCount;
    }

    inline SubFileReader::SubFileReader(MainFileReader& p_MainFile, const usize p_Offset, const usize p_Size)
        : m_MainFile(p_MainFile), m_Offset(p_Offset), m_Size(p_Size)
    {
        m_MainFile.addRef();
        if (p_Offset + p_Size > m_MainFile.getFileSize())
        {
            throw std::runtime_error("Subfile size exceeds main file size");
        }
    }

    inline SubFileReader::~SubFileReader()
    {
       release();
    }

    template <typename T>
    u32 SubFileReader::read(T& p_Value, const usize p_Offset)
    {
        if (p_Offset != UINT64_MAX)
            m_InternalOffset = p_Offset;

        const u32 l_MainFileOffset = m_MainFile.getCurrentPosition();

        const u32 l_CurrentOffset = m_InternalOffset;
        m_InternalOffset += m_MainFile.read(p_Value, m_Offset + m_InternalOffset);

        m_MainFile.setCurrentPosition(l_MainFileOffset);
        return m_InternalOffset - l_CurrentOffset;
    }

    template <typename T>
    u32 SubFileReader::readBytes(T* p_Buffer, const usize p_Size, const usize p_Offset)
    {
        if (p_Offset != UINT64_MAX)
            m_InternalOffset = p_Offset;

        const u32 l_MainFileOffset = m_MainFile.getCurrentPosition();

        const u32 l_CurrentOffset = m_InternalOffset;
        m_InternalOffset += m_MainFile.readBytes(p_Buffer, p_Size, m_Offset + m_InternalOffset);

        m_MainFile.setCurrentPosition(l_MainFileOffset);
        return m_InternalOffset - l_CurrentOffset;
    }

    template <typename T>
    u32 SubFileReader::readSpan(std::span<T> p_Vector, const usize p_Size, const usize p_Offset)
    {
        if (p_Offset != UINT64_MAX)
            m_InternalOffset = p_Offset;

        const u32 l_MainFileOffset = m_MainFile.getCurrentPosition();

        const u32 l_CurrentOffset = m_InternalOffset;
        m_InternalOffset += m_MainFile.readSpan(p_Vector, p_Size, m_Offset + m_InternalOffset);

        m_MainFile.setCurrentPosition(l_MainFileOffset);
        return m_InternalOffset - l_CurrentOffset;
    }

    inline usize SubFileReader::getFileSize() const
    {
        return m_Size;
    }

    inline usize SubFileReader::getCurrentPosition() const
    {
        return m_InternalOffset;
    }

    inline void SubFileReader::setCurrentPosition(const u32 p_Position)
    {
        m_InternalOffset = p_Position;
        if (m_InternalOffset > m_Size)
        {
            throw std::runtime_error("Subfile position exceeds subfile size");
        }
    }

    inline void SubFileReader::release()
    {
        if (m_Released)
            return;
        m_MainFile.release();
        m_Released = true;
    }
}
