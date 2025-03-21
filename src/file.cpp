#include <cstdio>
#include <cstring>

#include <unistd.h>

#include <sys/fcntl.h>

#include "file.h"

namespace subprocess {

File::File() : fp_(nullptr), is_owner_(false), mode_(FileMode::NONE) {}

File::File(int fd, bool is_owner, FileMode mode) : is_owner_(is_owner) {
    open(fd, is_owner, mode);
}

File::File(FILE* fp, bool is_owner) : is_owner_(is_owner) {
    open(fp, is_owner);
}

File::File(const std::filesystem::path& file, FileMode mode) {
    open(file, mode);
}

int File::fileno() const {
    return ::fileno(fp_.get());
}

FileMode File::filemode() const {
    return mode_;
}

FILE* File::filepointer() const {
    return fp_.get();
}

bool File::is_owner() const {
    return is_owner_;
}

bool File::is_opened() const {
    return fp_ != nullptr;
}

bool File::is_readable() const {
    return is_readable_filemode(mode_);
}

bool File::is_writable() const {
    return is_writable_filemode(mode_);
}

void File::set_bufsize(size_t size, int mode) {
    if (!is_opened()) {
        throw std::runtime_error("ERROR::SUBPROCESS::FILE: File is not opened.");
    }

    ::setvbuf(filepointer(), nullptr, mode, size);
}

void File::open(int fd, bool is_owner, FileMode mode) {
    is_owner_ = is_owner;

    if (mode == FileMode::NONE) {
        int flags = fcntl(fd, F_GETFL);
        if (flags == -1) {
            throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::FILE: Failed to get flags from file descriptor."); 
        }
        mode_ = flags_to_filemode(flags);
    } else {
        mode_ = mode;
    }

    FILE* fp = fdopen(fd, filemode_to_string(mode_).c_str());
    if (fp == nullptr) {
        throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::FILE: Failed to open file from file descriptor."); 
    }

    if (is_owner_) {
        fp_.reset(fp, [] (FILE* file) { fclose(file); });
    } else {
        fp_.reset(fp, [] (FILE* file) {});
    }
}

void File::open(FILE* fp, bool is_owner) {
    is_owner_ = is_owner;

    int fd = ::fileno(fp);
    if (fd == -1) {
        throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::FILE: Failed to retrieve file descriptor."); 
    }
    
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::FILE: Failed to get flags from file descriptor."); 
    }

    mode_ = flags_to_filemode(flags);

    if (fp == nullptr) {
        throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::FILE: Failed to open file from file descriptor."); 
    } 

    if (is_owner_) {
        fp_.reset(fp, [] (FILE* file) { fclose(file); });
    } else {
        fp_.reset(fp, [] (FILE* file) {});
    }
}

void File::open(const std::filesystem::path& file, FileMode mode) {
    if (!std::filesystem::exists(file)) {
        throw std::runtime_error("ERROR::SUBPROCESS::FILE: File " + file.string() + " is not exist.");
    }

    FILE* fp = fopen(file.c_str(), filemode_to_string(mode).c_str());
    if (fp == nullptr) {
        throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::FILE: Failed to open " + file.string() + ".");
    }

    fp_.reset(fp, [] (FILE* file) { fclose(file); });
    is_owner_ = true;
    mode_     = mode;
}

void File::close() {
    if (!is_opened()) {
        return;
    }

    if (is_owner()) {
        fp_       = nullptr;
        is_owner_ = false;
        mode_     = FileMode::NONE;
    } else {
        fclose(fp_.get());
        throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS:FILE: Failed to close file.");
        is_owner_ = false;
        mode_     = FileMode::NONE;
    }
}

void File::release() {
    fp_       = nullptr;
    is_owner_ = false;
    mode_     = FileMode::NONE;
}

Bytes File::read(Bytes::size_type n) const {
    if (!is_opened()) {
        throw std::runtime_error("ERROR::SUBPROCESS::FILE: File is not opened.");
    }

    Bytes buf;
    size_t total_size = 0;
    if (n == static_cast<Bytes::size_type>(-1)) {
        buf.resize(DEFAULT_BUF_SIZE);
        while (true) {
            if (total_size >= buf.size()) {
                buf.resize(2 * buf.size());
            }

            size_t read_size = std::fread(buf.data() + total_size, sizeof(Bytes::value_type), buf.size() - total_size, filepointer());
            total_size += read_size;
            if (read_size < buf.size() - total_size) {
                if (std::feof(filepointer())) {
                    break;
                } else if (std::ferror(filepointer())) {
                    throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::FILE: Failed to read from file."); 
                } else {
                    throw std::runtime_error("ERROR::SUBPROCESS::FILE: Failed to read from file.");
                }
            }
        }
    } else {
        buf.resize(n);
        while (total_size < buf.size()) {
            size_t read_size = std::fread(buf.data() + total_size, sizeof(Bytes::value_type), buf.size() - total_size, filepointer());
            total_size += read_size;
            if (read_size < buf.size() - total_size) {
                if (std::feof(filepointer())) {
                    break;
                } else if (std::ferror(filepointer())) {
                    throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::FILE: Failed to read from file."); 
                } else {
                    throw std::runtime_error("ERROR::SUBPROCESS::FILE: Failed to read from file.");
                }
            }
        } 
    }

    buf.resize(total_size);
    return buf;
}

Bytes File::readline(Bytes::size_type n) const {

    if (!is_opened()) {
        throw std::runtime_error("ERROR::SUBPROCESS::FILE: File is not opened.");
    }

    Bytes buf;
    if (n == static_cast<Bytes::size_type>(-1)) {
        size_t total_size = 0;
        buf.resize(DEFAULT_BUF_SIZE);
        while (true) {
            if (total_size + 1 >= buf.size()) {
                buf.resize(2 * buf.size());
            }

            if (std::fgets(buf.data() + total_size, buf.size() - total_size, filepointer()) == nullptr) {
                if (std::feof(filepointer())) {
                    break;
                } else {
                    throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::FILE: Failed to read from file.");
                }
            }

            size_t read_size = std::strlen(buf.data() + total_size);
            total_size += read_size;
            if (read_size != std::strcspn(buf.data() + total_size - read_size, "\r\n")) {
                break;
            }
        }
    } else {
        buf.resize(n + 1);
        if (std::fgets(buf.data(), buf.size(), filepointer()) == nullptr) {
            if (std::ferror(filepointer())) {
                throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::FILE: Failed to read from file.");
            }
        }
    }

    buf.resize(strlen(buf.data()));
    return buf;
}

void File::write(const Bytes& bytes) {
    if (!is_opened()) {
        throw std::runtime_error("ERROR::SUBPROCESS::FILE: File is not opened.");
    }

    size_t total_size = 0;
    while (total_size < bytes.size()) {
        size_t written_size = std::fwrite(bytes.data() + total_size, sizeof(Bytes::value_type), bytes.size() - total_size, filepointer());
        if (written_size < bytes.size() - total_size) {
            if (std::feof(filepointer())) {
                break;
            } else {
                throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::FILE: Failed to write to file."); 
            }
        }
        total_size += written_size;
    }
    std::fflush(filepointer());
}

}