#include <future>
#include <stdexcept>
#include <thread>
#include <utility>

#include <fcntl.h>
#include <unistd.h>

#include "exception.h"
#include "streamable.h"

namespace subprocess {

/* ===================================== File ===================================== */

File::File() : fp_(nullptr) {}
File::File(int fd) {
    int flags = ::fcntl(fd, F_GETFL);
    if (flags == -1)
        throw OSError(errno, std::generic_category(), "Failed to retrieve file status flags using fcntl");

    if      ((flags & O_ACCMODE) == O_RDONLY)
        fp_ = ::fdopen(fd, "r");
    else if ((flags & O_ACCMODE) == O_WRONLY) 
        fp_ = ::fdopen(fd, "w");
    else if ((flags & O_ACCMODE) == O_RDWR)
        fp_ = ::fdopen(fd, "r+");
    else
        throw std::runtime_error("Unsupported file access mode.");

    if (fp_ == nullptr)
        throw std::runtime_error("Failed to open file descriptor.");
}
File::File(FILE* fp) { open(fp); }
File::File(const File& other) : fp_(other.fp_) {}
File::File(File&& other) noexcept : fp_(std::exchange(other.fp_, nullptr)) {}

File& File::operator=(const File& other) { 
    fp_ = other.fp_; 
    return *this;
}
File& File::operator=(File&& other) noexcept { 
    fp_ = std::exchange(other.fp_, nullptr);
    return *this;
}

int File::fileno() const { 
    if (!is_opened())
        return -1;
    return ::fileno(fp_); 
}

bool File::is_opened() const { return fp_ != nullptr; }
bool File::is_readable() const {
    if (!is_opened())
        return false;

    int flags = ::fcntl(fileno(), F_GETFL);
    if (flags == -1) 
        throw OSError(errno, std::generic_category(), "Failed to retrieve file status flags using fcntl");

    /** Extract the access mode bits (last two bits) using O_ACCMODE(=0b11) mask.
     *  The possible values are:
     * - O_RDONLY (=0b00) → Read-only mode
     * - O_WRONLY (=0b01) → Write-only mode (not readable)
     * - O_RDWR   (=0b10) → Read-write mode */
    return ((flags & O_ACCMODE) == O_RDONLY) ||
           ((flags & O_ACCMODE) & O_RDWR);
}
bool File::is_writable() const {
    if (!is_opened())
        return false;

    int flags = ::fcntl(fileno(), F_GETFL);
    if (flags == -1) 
        throw OSError(errno, std::generic_category(), "Failed to retrieve file status flags using fcntl");
    /** Extract the access mode bits (last two bits) using O_ACCMODE(=0b11) mask.
     *  The possible values are:
     * - O_RDONLY (=0b00) → Read-only mode
     * - O_WRONLY (=0b01) → Write-only mode (not readable)
     * - O_RDWR   (=0b10) → Read-write mode */
    return ((flags & O_ACCMODE) == O_WRONLY) ||
           ((flags & O_ACCMODE) == O_RDWR);
}

Bytes File::read(Bytes::size_type size) {
    if (!is_opened())
        throw std::runtime_error("Attempted to read from a closed file.");
    if (!is_readable())
        throw std::runtime_error("File is not readable.");

    Bytes buf(size);
    size_t total_bytes = 0;
    while (total_bytes < buf.size()) {
        size_t bytes_to_read = buf.size() - total_bytes;
        size_t bytes_read = std::fread(buf.c_str() + total_bytes, sizeof(Bytes::value_type), bytes_to_read, fp_);
        total_bytes += bytes_read;
        if (bytes_read < bytes_to_read) {
            if (std::feof(fp_)) {
                break;
            } else {
                throw std::runtime_error("Error occurred while reading from the file.");
            }
        }
    }
    buf.resize(total_bytes);
    return buf;
}

Bytes File::read_all() {
    if (!is_opened())
        throw std::runtime_error("Attempted to read from a closed file.");
    if (!is_readable())
        throw std::runtime_error("File is not readable.");

    Bytes buf(BUFSIZ);
    size_t total_bytes = 0;
    while (true) {
        if (buf.size() <= total_bytes)
            buf.resize(buf.size() * 2);
        size_t bytes_to_read = buf.size() - total_bytes;
        size_t bytes_read = std::fread(buf.c_str() + total_bytes, sizeof(Bytes::value_type), bytes_to_read, fp_);
        total_bytes += bytes_read;
        if (bytes_read < bytes_to_read) {
            if (std::feof(fp_)) {
                break;
            } else {
                throw std::runtime_error("Error occurred while reading from the file.");
            }
        }
    }
    buf.resize(total_bytes);
    return buf;
}

Bytes::size_type File::write(const Bytes& buf, Bytes::size_type size) {
    if (!is_opened())
        throw std::runtime_error("Attempted to write to a closed file.");
    if (!is_writable())
        throw std::runtime_error("File is not writable.");

    size_t total_bytes = 0;
    while (total_bytes < size) {
        size_t bytes_to_write = size - total_bytes;
        size_t bytes_written = std::fwrite(buf.c_str() + total_bytes, sizeof(Bytes::value_type), bytes_to_write, fp_);
        total_bytes += bytes_written;
        if (bytes_written < bytes_to_write) {
            if (ferror(fp_)) {
                throw std::runtime_error("Error occurred while writing to the file."); 
            }
        }
    }
    fflush(fp_);
    return total_bytes;
}

void File::close() { 
    if (is_opened() && fclose(fp_) == -1)
        throw OSError(errno, std::generic_category(), "Failed to close the file");
}
void File::release() { fp_ = nullptr; }

void File::open(FILE* fp) { fp_ = fp; }

void File::set_bufsize(ssize_t size) {
    int ret;
    if (size == 0)
        ret = ::setvbuf(fp_, nullptr, _IONBF, BUFSIZ);
    else if (size == 1)
        ret = ::setvbuf(fp_, nullptr, _IOLBF, BUFSIZ);
    else if (size > 0)
        ret = ::setvbuf(fp_, nullptr, _IOFBF, size);
    else 
        ret = ::setvbuf(fp_, nullptr, _IOFBF, BUFSIZ);

    if (ret == -1)
        throw OSError(errno, std::generic_category(), "Failed to set buffer size");
}

/* ===================================== IStream ===================================== */

IStream::IStream() : stream_(nullptr) {}
IStream::IStream(std::istream* stream) { open(stream); }
IStream::IStream(const IStream& other) : stream_(other.stream_) {}
IStream::IStream(IStream&& other) noexcept : stream_(std::exchange(other.stream_, nullptr)) {}

IStream& IStream::operator=(const IStream& other) { 
    if (this != &other)
        stream_ = other.stream_; 
    return *this;
}
IStream& IStream::operator=(IStream&& other) noexcept { 
    stream_ = std::exchange(other.stream_, nullptr); 
    return *this;
}

int IStream::fileno() const { return -1; }

bool IStream::is_opened() const { return stream_ != nullptr && stream_->good(); }
bool IStream::is_readable() const { return is_opened() && !stream_->eof(); }
bool IStream::is_writable() const { return false; }

Bytes IStream::read(Bytes::size_type size) {
    if (!is_opened())
        throw std::runtime_error("Attempted to read from a closed stream.");
    if (!is_readable())
        throw std::runtime_error("Stream is not readable.");

    Bytes buf(size);
    stream_->read(buf.c_str(), size);
    buf.resize(stream_->gcount());
    return buf;
}

Bytes IStream::read_all() {
    if (!is_opened())
        throw std::runtime_error("Attempted to read from a closed stream.");
    if (!is_readable())
        throw std::runtime_error("Stream is not readable.");

    Bytes buf(BUFSIZ);
    std::streamsize total_bytes = 0;
    while (true) {
        if (buf.size() <= total_bytes)
            buf.resize(buf.size() * 2);
        std::streamsize bytes_to_read = buf.size() - total_bytes;
        stream_->read(buf.c_str() + total_bytes, bytes_to_read);
        total_bytes += stream_->gcount();
        if (stream_->eof()) // Operations that reach the End-of-File may also set the failbit, so EOF should be checked first
            break;          // https://stackoverflow.com/questions/70306575/why-is-failbit-set-when-i-enter-eof
        if (stream_->fail())
            throw std::runtime_error("Error occurred while reading from the stream.");
   }
   buf.resize(total_bytes);
   return buf;
}

void IStream::close() { stream_ = nullptr; }
void IStream::release() { stream_ = nullptr; }
void IStream::open(std::istream* stream) { stream_ = stream; }

/* ===================================== OStream ===================================== */

OStream::OStream() : stream_(nullptr) {}
OStream::OStream(std::ostream* stream) { open(stream); }
OStream::OStream(const OStream& other) : stream_(other.stream_) {}
OStream::OStream(OStream&& other) noexcept : stream_(std::exchange(other.stream_, nullptr)) {}

OStream& OStream::operator=(const OStream& other) { 
    if (this != &other)
        stream_ = other.stream_; 
    return *this;
}
OStream& OStream::operator=(OStream&& other) noexcept { 
    stream_ = std::exchange(other.stream_, nullptr); 
    return *this;
}

int OStream::fileno() const { return -1; }

bool OStream::is_opened() const { return stream_ != nullptr && stream_->good(); }
bool OStream::is_readable() const { return false; }
bool OStream::is_writable() const { return is_opened(); } 

Bytes::size_type OStream::write(const Bytes& buf, Bytes::size_type size) {
    if (!is_opened())
        throw std::runtime_error("Attempted to write to a closed stream.");
    if (!is_writable())
        throw std::runtime_error("Stream is not writable.");

    stream_->write(buf.data(), size);
    if (stream_->fail())
        throw std::runtime_error("Error occurred while writing to the stream.");
    stream_->flush();
    return size;
}

void OStream::close() { stream_ = nullptr; }
void OStream::release() { stream_ = nullptr; }
void OStream::open(std::ostream* stream) { stream_ = stream; }

/* ===================================== IOStream ===================================== */

IOStream::IOStream() : stream_(nullptr) {}
IOStream::IOStream(std::iostream* stream) { open(stream); }
IOStream::IOStream(const IOStream& other) : stream_(other.stream_) {}
IOStream::IOStream(IOStream&& other) noexcept : stream_(std::exchange(other.stream_, nullptr)) {}

IOStream& IOStream::operator=(const IOStream& other) { 
    if (this != &other)
        stream_ = other.stream_; 
    return *this;
}
IOStream& IOStream::operator=(IOStream&& other) noexcept { 
    stream_ = std::exchange(other.stream_, nullptr); 
    return *this;
}

int IOStream::fileno() const { return -1; }

bool IOStream::is_opened() const { return stream_ != nullptr && stream_->good(); }
bool IOStream::is_readable() const { return is_opened() && !stream_->eof(); }
bool IOStream::is_writable() const { return is_opened(); }

Bytes IOStream::read(Bytes::size_type size) {
    if (!is_opened())
        throw std::runtime_error("Attempted to read from a closed stream.");
    if (!is_readable())
        throw std::runtime_error("Stream is not readable.");

    Bytes buf(size);
    stream_->read(buf.c_str(), size);
    buf.resize(stream_->gcount());
    return buf;
}

Bytes IOStream::read_all() {
    if (!is_opened())
        throw std::runtime_error("Attempted to read from a closed stream.");
    if (!is_readable())
        throw std::runtime_error("Stream is not readable.");

    Bytes buf(BUFSIZ);
    std::streamsize total_bytes = 0;
    while (true) {
        if (buf.size() <= total_bytes)
            buf.resize(buf.size() * 2);
        std::streamsize bytes_to_read = buf.size() - total_bytes;
        stream_->read(buf.c_str() + total_bytes, bytes_to_read);
        total_bytes += stream_->gcount();
        if (stream_->eof()) // Operations that reach the End-of-File may also set the failbit, so EOF should be checked first
            break;          // https://stackoverflow.com/questions/70306575/why-is-failbit-set-when-i-enter-eof
        if (stream_->fail())
            throw std::runtime_error("Error occurred while reading from the stream.");
   }
   buf.resize(total_bytes);
   return buf;
}

Bytes::size_type IOStream::write(const Bytes& buf, Bytes::size_type size) {
    if (!is_opened())
        throw std::runtime_error("Attempted to write to a closed stream.");
    if (!is_writable())
        throw std::runtime_error("Stream is not writable.");

    stream_->write(buf.data(), size);
    if (stream_->fail())
        throw std::runtime_error("Error occurred while writing to the stream.");
    stream_->flush();
    return size;
}

void IOStream::close() { stream_ = nullptr; }
void IOStream::release() { stream_ = nullptr; }
void IOStream::open(std::iostream* stream) { stream_ = stream; }

/* ===================================== Functions ===================================== */

Bytes::size_type communicate(IStreamable& in, OStreamable& out) {
    if (!in.is_opened())
        throw std::runtime_error("Attempted to read from a closed stream.");
    if (!out.is_opened())
        throw std::runtime_error("Attempted to write to a closed stream.");
    if (!in.is_readable())
        throw std::runtime_error("Stream is not readable.");
    if (!out.is_writable())
        throw std::runtime_error("Stream is not writable.");

    Bytes bytes = in.read_all();
    return out.write(bytes, bytes.size());
}

std::future<Bytes::size_type> communicate_async(IStreamable& in, OStreamable& out) {
    if (!in.is_opened())
        throw std::runtime_error("Attempted to read from a closed stream.");
    if (!out.is_opened())
        throw std::runtime_error("Attempted to write to a closed stream.");
    if (!in.is_readable())
        throw std::runtime_error("Stream is not readable.");
    if (!out.is_writable())
        throw std::runtime_error("Stream is not writable.");

    return std::async(std::launch::async, [&]() {
         Bytes bytes = in.read_all();
         return out.write(bytes, bytes.size());
    });
}

} // namespace subprocess