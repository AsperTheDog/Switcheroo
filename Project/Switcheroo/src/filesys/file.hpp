#pragma once
#include "../util/common.hpp"

#include <filesystem>
#include <fstream>
#include <span>

namespace swroo {
    class FileReader
    {
    public:
        virtual ~FileReader() = default;

        template<typename T>
        u32 read(T& p_Value, usize p_Offset = UINT64_MAX);

        template<typename T>
        u32 readData(T* p_Buffer, usize p_Size, usize p_Offset = UINT64_MAX);

        template<typename T>
        u32 readSpan(std::span<T> p_Vector, usize p_Size, usize p_Offset = UINT64_MAX);

        virtual u32 readBytes(u8* p_Buffer, usize p_Size, usize p_NewOffset) = 0;

        [[nodiscard]] virtual usize getFileSize() const = 0;
        [[nodiscard]] virtual usize getCurrentPosition() = 0;
        [[nodiscard]] virtual usize getCurrentGlobalPosition() = 0;
        [[nodiscard]] virtual std::filesystem::path getFilePath() const = 0;

        virtual void setCurrentPosition(usize p_Position) = 0;

        virtual void addRef() = 0;
        virtual void release() = 0;

        virtual bool isOpen() = 0;

    protected:
        u32 m_References = 0;
    };

    class MainFileReader final : public FileReader
    {
    public:
        explicit MainFileReader(const std::filesystem::path& p_File);
        ~MainFileReader() override = default;

        [[nodiscard]] usize getFileSize() const override;
        [[nodiscard]] usize getCurrentPosition() override;
        [[nodiscard]] usize getCurrentGlobalPosition() override { return getCurrentPosition(); }
        [[nodiscard]] std::filesystem::path getFilePath() const override { return m_FilePath; }

        void setCurrentPosition(usize p_Position) override;

        void addRef() override;
        void release() override;

        bool isOpen() override { return m_File.is_open(); }

    private:
        u32 readBytes(u8* p_Buffer, usize p_Size, usize p_NewOffset) override;

    private:
        std::ifstream m_File;
        std::filesystem::path m_FilePath;
        usize m_FileSize = 0;
    };

    class SubFileReader final : public FileReader
    {
    public:
        explicit SubFileReader(FileReader& p_MainFile, usize p_Offset, usize p_Size);
        ~SubFileReader() override;

        [[nodiscard]] usize getFileSize() const override;
        [[nodiscard]] usize getCurrentPosition() override;
        [[nodiscard]] usize getCurrentGlobalPosition() override { return m_Offset + getCurrentPosition(); }
        [[nodiscard]] std::filesystem::path getFilePath() const override { return m_ParentFile.getFilePath(); }

        void setCurrentPosition(usize p_Position) override;

        void addRef() override;
        void release() override;

        bool isOpen() override { return m_Released; }

    private:
        u32 readBytes(u8* p_Buffer, usize p_Size, usize p_NewOffset) override;

        FileReader& m_ParentFile;
        usize m_Offset = 0;
        usize m_Size = 0;

        usize m_InternalOffset = 0;

        bool m_Released = false;
    };

    template <typename T>
    u32 FileReader::read(T& p_Value, const usize p_Offset)
    {
        return readBytes(reinterpret_cast<u8*>(&p_Value), sizeof(T), p_Offset);
    }

    template <typename T>
    u32 FileReader::readData(T* p_Buffer, const usize p_Size, const usize p_Offset)
    {
        return readBytes(reinterpret_cast<u8*>(p_Buffer), p_Size, p_Offset);
    }

    template <typename T>
    u32 FileReader::readSpan(std::span<T> p_Vector, const usize p_Size, const usize p_Offset)
    {
        if (p_Vector.size() < p_Size)
            throw std::runtime_error("Buffer size is smaller than requested size");
        
        return readBytes(reinterpret_cast<u8*>(p_Vector.data()), p_Size * sizeof(T), p_Offset);
    }

    inline MainFileReader::MainFileReader(const std::filesystem::path& p_File)
    {
        m_FilePath = p_File;
        m_File.open(m_FilePath, std::ios::binary | std::ios::ate);
        if (!m_File.is_open())
            throw std::runtime_error("Failed to open file: " + m_FilePath.string());
        
        m_FileSize = static_cast<usize>(MainFileReader::getCurrentPosition());
        m_File.seekg(0, std::ios::beg);

        m_References = 1;
    }

    inline usize MainFileReader::getFileSize() const
    {
        return m_FileSize;
    }

    inline usize MainFileReader::getCurrentPosition()
    {
        return m_File.tellg();
    }

    inline void MainFileReader::setCurrentPosition(const usize p_Position)
    {
        m_File.seekg(p_Position);
        if (m_File.fail())
            throw std::runtime_error("Failed to set file position: " + m_FilePath.string());
    }

    inline void MainFileReader::release()
    {
        m_References--;

        if (m_References == 0)
            m_File.close();
    }

    inline void MainFileReader::addRef()
    {
        if (m_References == 0)
            m_File.open(m_FilePath, std::ios::binary);

        m_References++;
    }

    inline u32 MainFileReader::readBytes(u8* p_Buffer, const usize p_Size, const usize p_NewOffset)
    {
        if (p_NewOffset != UINT64_MAX)
            setCurrentPosition(p_NewOffset);
        
        m_File.read(reinterpret_cast<char*>(p_Buffer), p_Size);
        if (m_File.fail())
            throw std::runtime_error("Failed to read file: " + m_FilePath.string());
        
        return static_cast<u32>(m_File.gcount());
    }

    inline SubFileReader::SubFileReader(FileReader& p_MainFile, const usize p_Offset, const usize p_Size)
        : m_ParentFile(p_MainFile), m_Offset(p_Offset), m_Size(p_Size)
    {
        m_ParentFile.addRef();
        if (p_Offset + p_Size > m_ParentFile.getFileSize())
        {
            throw std::runtime_error("Subfile size exceeds main file size");
        }
    }

    inline SubFileReader::~SubFileReader()
    {
        SubFileReader::release();
    }

    inline usize SubFileReader::getFileSize() const
    {
        return m_Size;
    }

    inline usize SubFileReader::getCurrentPosition()
    {
        return m_InternalOffset;
    }

    inline void SubFileReader::setCurrentPosition(const usize p_Position)
    {
        if (p_Position > m_Size)
            throw std::runtime_error("Subfile position exceeds subfile size");
        
        m_InternalOffset = p_Position;
    }

    inline void SubFileReader::release()
    {
        if (m_Released)
            return;
        m_References--;
        if (m_References == 0)
        {
            m_ParentFile.release();
            m_Released = true;
        }
    }

    inline void SubFileReader::addRef()
    {
        if (m_Released)
        {
            m_ParentFile.addRef();
            m_Released = false;
        }
        m_References++;
    }

    inline u32 SubFileReader::readBytes(u8* p_Buffer, const usize p_Size, const usize p_NewOffset)
    {
        if (p_NewOffset != UINT64_MAX)
            setCurrentPosition(p_NewOffset);

        const u32 l_ReadSize = m_ParentFile.readBytes(p_Buffer, p_Size, m_Offset + m_InternalOffset);
        m_InternalOffset += l_ReadSize;
        return l_ReadSize;
    }
}
