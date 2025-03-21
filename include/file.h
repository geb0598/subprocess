#ifndef FILE_H
#define FILE_H

#include <filesystem>
#include <string>
#include <vector>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "bytes.h"

namespace {
namespace subprocess {

enum class FileMode {
    NONE,
    READ,
    READWRITE,
    WRITE,
    WRITEREAD,
    APPEND,
    APPENDREAD,
    READ_BYTE,
    READWRITE_BYTE,
    WRITE_BYTE,
    WRITEREAD_BYTE,
    APPEND_BYTE,
    APPENDREAD_BYTE
};

inline std::string filemode_to_string(FileMode mode) {
    switch (mode) {
        case FileMode::READ:
            return "r";
        case FileMode::READWRITE:
            return "r+";
        case FileMode::WRITE:
            return "w";
        case FileMode::WRITEREAD:
            return "w+";
        case FileMode::APPEND:
            return "a";
        case FileMode::APPENDREAD:
            return "a+";
        case FileMode::READ_BYTE:
            return "rb";
        case FileMode::READWRITE_BYTE:
            return "rb+";
        case FileMode::WRITE_BYTE:
            return "wb";
        case FileMode::WRITEREAD_BYTE:
            return "wb+";
        case FileMode::APPEND_BYTE:
            return "ab";
        case FileMode::APPENDREAD_BYTE:
            return "ab+";
        default:
            throw std::runtime_error("ERROR::SUBPROCESS: Invalid file mode.");
    }
}

inline FileMode flags_to_filemode(int flags) {
    if (flags & O_APPEND) {
        if (flags & O_WRONLY) {
            return FileMode::APPEND_BYTE;
        } else if (flags & O_RDWR) {
            return FileMode::APPENDREAD_BYTE;
        } else {
            throw std::runtime_error("ERROR::SUBPROCESS::FILE: Unknown file descriptor flag " + std::to_string(flags) + ".");
        }
    } else {
        if (flags & O_WRONLY) {
            return FileMode::WRITE_BYTE;
        } else if (flags & O_RDWR) {
            return FileMode::READWRITE_BYTE;
        } else {
            return FileMode::READ_BYTE;
        }
    }
}

inline bool is_readable_filemode(FileMode mode) {
    return mode == FileMode::READ            ||
           mode == FileMode::READWRITE       ||
           mode == FileMode::WRITEREAD       ||
           mode == FileMode::APPENDREAD      ||
           mode == FileMode::READ_BYTE       ||
           mode == FileMode::READWRITE_BYTE  ||
           mode == FileMode::WRITEREAD_BYTE  ||
           mode == FileMode::APPENDREAD_BYTE;
}

inline bool is_writable_filemode(FileMode mode) {
    return mode == FileMode::READWRITE       ||
           mode == FileMode::WRITE           ||
           mode == FileMode::WRITEREAD       ||
           mode == FileMode::APPEND          ||
           mode == FileMode::APPENDREAD      ||
           mode == FileMode::READWRITE_BYTE  ||
           mode == FileMode::WRITE_BYTE      ||
           mode == FileMode::WRITEREAD_BYTE  ||
           mode == FileMode::APPEND_BYTE     ||
           mode == FileMode::APPENDREAD_BYTE;
}

class File {
public:
    ~File() = default;
    File();
    // TODO: Unable to extract file information from fcntl, because it doesn't distinguish text mode and binary mode.
    File(FILE* fp, bool is_owner = false);
    File(int fd, bool is_owner = false, FileMode mode = FileMode::NONE);
    File(const std::filesystem::path& file, FileMode mode);

    File(const File& other)     = default;
    File(File&& other) noexcept = default;

    File&    operator=(const File& other)     = default;
    File&    operator=(File&& other) noexcept = default;

    int      fileno() const;
    FileMode filemode() const;
    FILE*    filepointer() const;

    bool     is_owner() const;
    bool     is_opened() const;
    bool     is_readable() const;
    bool     is_writable() const;

    void     set_bufsize(size_t size, int mode);

    void     open(FILE* fp, bool is_owner = false);
    void     open(int fd, bool is_owner = false, FileMode mode = FileMode::NONE);
    void     open(const std::filesystem::path& file, FileMode mode);
    void     close();
    void     release();

    Bytes    read(Bytes::size_type n = -1) const;
    Bytes    readline(Bytes::size_type n = -1) const;
    void     write(const Bytes& bytes);

private:
    static constexpr Bytes::size_type DEFAULT_BUF_SIZE = 1024;

    std::shared_ptr<FILE> fp_;
    FileMode              mode_;
    bool                  is_owner_;
};

}

}

#endif