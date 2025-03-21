#ifndef STREAMABLE_H
#define STREAMABLE_H

#include <cstdio>
#include <future>
#include <iostream>
#include <thread>

#include "bytes.h"

namespace subprocess {


/* ===================================== Interfaces ===================================== */

/** @brief Abstract interface for streamable objects.
 *
 *  This interface provides a common API for non-owning wrappers around stream-like 
 *  resources (e.g., file descriptors, FILE* handles, or C++ streams), inspired by 
 *  the C++ standard library (e.g., std::ios, std::istream, std::ostream). It does 
 *  not manage resource ownership, so the caller is responsible for initialization 
 *  and proper closure of the underlying resource.
 */
class Streamable {
public:
    virtual ~Streamable()            = default;

    /** @brief Retrieves the file descriptor associated with this object.
     *  @return File descriptor if available, otherwise -1.
     */
    virtual int fileno() const       = 0;

    /** @brief Checks if the object is associated with a valid resource.
     *  @return True if linked to a resource, false otherwise.
     *  @note This does not guarantee that the resource is actually open.
     */
    virtual bool is_opened() const   = 0;
    virtual bool is_readable() const = 0;
    virtual bool is_writable() const = 0;

    /** @brief Closes the associated resource and releases it. */
    virtual void close()             = 0;

    /** @brief Releases the association with the resource without closing it. */
    virtual void release()           = 0;
};

/** @brief Interface for readable stream-like objects. */
class IStreamable : virtual public Streamable {
public:
    virtual ~IStreamable() = default;

    /** @brief Reads a specified number of bytes from the stream.
     *  
     *  The function attempts to read up to the given size but may return
     *  fewer bytes if EOF is reached or if interrupted. If the stream
     *  is not readable or an error occurs, an exception is thrown.
     *
     *  @param size The number of bytes to read.
     *  @return A Bytes object containing the data read.
     *  @throws std::runtime_error If the stream is not readable or an error occurs.
     */
    virtual Bytes read(Bytes::size_type size) = 0;
    virtual Bytes read_all()                  = 0;
};

/** @brief Interface for writable stream-like objects. */
class OStreamable : virtual public Streamable {
public:
    virtual ~OStreamable() = default;

    virtual Bytes::size_type write(const Bytes& bytes, Bytes::size_type size) = 0;
};

/** @brief Interface for stream-like objects that support both reading and writing.
 *
 *  This interface provides an abstraction for objects that can be used for both input 
 *  and output operations, such as FILE*, std::iostream, etc. It currently does not contain 
 *  any methods, but may be extended in the future to include additional functionality.
 */
class IOStreamable : public IStreamable, public OStreamable {
public:
    virtual ~IOStreamable() = default;
};

/* ===================================== Classes ===================================== */

/** @brief A lightweight, non-owning wrapper for `FILE*` */
class File : public IOStreamable {
public:
    ~File() = default;
    File();
    File(int fd);
    File(FILE* fp);
    File(const File& other);
    File(File&& other) noexcept;

    File&                    operator=(const File& other);
    File&                    operator=(File&& other) noexcept;

    virtual int              fileno() const override;

    virtual bool             is_opened() const override;
    virtual bool             is_readable() const override;
    virtual bool             is_writable() const override;

    virtual Bytes            read(Bytes::size_type size) override;
    virtual Bytes            read_all() override;
    virtual Bytes::size_type write(const Bytes& buf, Bytes::size_type size) override;

    virtual void             close() override;
    virtual void             release() override;
 
    void                     open(FILE* fp);

    /** @brief Configures the buffering mode of the file stream.
     *
     *  Sets the buffering mode and buffer size for the file stream.
     *  - `size == 0`: No buffering   (_IONBF)
     *  - `size == 1`: Line buffering (_IOLBF)
     *  - `size >  1`: Full buffering (_IOFBF) with the specified size
     *  - `size <  0`: Full buffering (_IOFBF) with default size
     *
     *  @param size The desired buffer size in bytes.
     *  @throws OSError If setting the buffer fails.
     */
    void                     set_bufsize(ssize_t size);
    void                     set_cloexec();

private:
    std::FILE*               fp_;
};

/** @brief A lightweight, non-owning wrapper for `std::istream` */
class IStream : public IStreamable {
public:
    virtual ~IStream() = default;
    IStream();
    IStream(std::istream* stream);
    IStream(const IStream& other);
    IStream(IStream&& other) noexcept;

    IStream&      operator=(const IStream& other);
    IStream&      operator=(IStream&& other) noexcept;

    virtual int   fileno() const override;

    /** @brief Returns true if an associated stream exists and is in a good state. */
    virtual bool  is_opened() const override;
    /** @brief Returns true if is_opened() is true and not at EOF. */
    virtual bool  is_readable() const override;
    virtual bool  is_writable() const override;

    virtual Bytes read(Bytes::size_type size) override;
    virtual Bytes read_all() override;

    /** @brief Detaches the stream without closing it (equivalent to release()). */
    virtual void  close() override;
    virtual void  release() override;

    void          open(std::istream* stream);

private:
    std::istream* stream_;
};

/** @brief A lightweight, non-owning wrapper for `std::ostream` */
class OStream : public OStreamable {
public:
    virtual ~OStream() = default;
    OStream();
    OStream(std::ostream* stream);
    OStream(const OStream& other);
    OStream(OStream&& other) noexcept;

    OStream&                 operator=(const OStream& other);
    OStream&                 operator=(OStream&& other) noexcept;

    virtual int              fileno() const override;

    /** @brief Returns true if an associated stream exists and is in a good state. */
    virtual bool             is_opened() const override;
    virtual bool             is_readable() const override;
    /** @brief Equivalent to is_opened() */
    virtual bool             is_writable() const override;

    virtual Bytes::size_type write(const Bytes& buf, Bytes::size_type size) override;

    /** @brief Detaches the stream without closing it (equivalent to release()). */
    virtual void             close() override;
    virtual void             release() override;

    void                     open(std::ostream* stream);

private:
    std::ostream*            stream_;
};

/** @brief A lightweight, non-owning wrapper for `std::iostream` */
class IOStream : public IOStreamable {
public:
    virtual ~IOStream() = default;
    IOStream();
    IOStream(std::iostream* stream);
    IOStream(const IOStream& other);
    IOStream(IOStream&& other) noexcept;

    IOStream&                operator=(const IOStream& other);
    IOStream&                operator=(IOStream&& other) noexcept;

    virtual int              fileno() const override;

    virtual bool             is_opened() const override;
    virtual bool             is_readable() const override;
    virtual bool             is_writable() const override;

    virtual Bytes            read(Bytes::size_type size) override;
    virtual Bytes            read_all() override;
    virtual Bytes::size_type write(const Bytes& buf, Bytes::size_type size) override;

    virtual void             close() override;
    virtual void             release() override;

    void                     open(std::iostream* stream);

private:
    std::iostream*           stream_;
};

// TODO: Add FStream inherited from IOStream. It overrides open and close behaviors.

/* ===================================== Functions ===================================== */

/** @brief Synchronously transfers data from the input stream to the output stream.
 *
 *  Reads all data from the input stream and writes it to the output stream.
 * 
 *  @param in The input stream (must be open and readable).
 *  @param out The output stream (must be open and writable).
 * 
 *  @return The number of bytes written to the output stream.
 * 
 *  @throws std::runtime_error if any stream is not open, readable, writable, or an error occurs during reading/writing.
 */
Bytes::size_type communicate(IStreamable& in, OStreamable& out);

/** @brief Asynchronously transfers data from the input stream to the output stream.
 *
 *  Initiates an asynchronous operation to read from the input stream and write to the output stream.
 * 
 *  @param in The input stream (must be open and readable).
 *  @param out The output stream (must be open and writable).
 * 
 *  @return A future containing the number of bytes written to the output stream.
 * 
 *  @throws std::runtime_error if any stream is not open, readable, writable, or an error occurs during reading/writing.
 * 
 *  @note Streamable objects are not thread-safe. Avoid using this function with Streamable objects 
 *        that may be associated with common resources.
 */
std::future<Bytes::size_type> communicate_async(IStreamable& in, OStreamable& out);

} // namespace subprocess

#endif