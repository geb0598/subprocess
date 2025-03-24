#ifndef TYPES_H
#define TYPES_H

#include <filesystem>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "streamable.h"

namespace subprocess {

namespace types {

/** @brief Represents command-line arguments for process execution.
 *
 *  This class is designed for temporary use to construct argument lists 
 *  when launching a process. It allows initialization with a variable 
 *  number of parameters, automatically storing them as a vector of strings.
 */
class args_t {
public:
    template<typename... Params>
    explicit args_t(Params&&... params) : args(concat(std::forward<Params>(params)...)) {}

    std::vector<std::string> args;
private:
    template<typename... Params>
    std::vector<std::string> concat(Params&&... params) { return {std::forward<Params>(params)...}; }
};

/** @brief Represents the buffer size for process pipe communication.
 *
 *  This class is used to specify the buffer size for pipes. 
 *  - `size == 0`  : No buffering
 *  - `size == 1`  : Line buffering
 *  - `size >  1` : Full buffering with the specified size
 *  - `size <  0` : Full buffering with a default size
 */
class bufsize_t {
public:
    explicit bufsize_t(ssize_t bufsize);
    ssize_t bufsize;
};

enum class IOOption { NONE, PIPE, STDOUT, DEVNULL };

/** @brief Represents the standard input source for a process.
 *
 *  This class defines how the standard input of a process is managed. It supports 
 *  various input sources, including files, streams, and pipes. The behavior depends 
 *  on the provided source:
 *
 *  - If no source is specified (`PIPE` option), the process can receive input 
 *    through `pipe_writer`, which represents the pipe’s write end.
 *  - If a source is available and a file descriptor can be obtained from it, 
 *    the source is directly connected to standard input using `::dup2`.
 *  - If a source is available but does not provide a file descriptor, 
 *    communication between the source and standard input is emulated using 
 *    a pipe and the `communicate` function.
 */
class std_in_t {
public:
    explicit std_in_t(int fd);
    explicit std_in_t(FILE* fp);
    explicit std_in_t(IOOption option);
    explicit std_in_t(std::istream* stream);
    explicit std_in_t(const std::filesystem::path& file);

    std::shared_ptr<File>        pipe_reader;
    std::shared_ptr<File>        pipe_writer;
    std::shared_ptr<IStreamable> source;

private:
    static void auto_close(IStreamable* stream) noexcept {
        if (!stream)
            return;
        try {
            stream->close();
        } catch (const std::exception& e) {
            perror("Failed to close the stream");
        }
        delete stream;
    }
};

/** @brief Represents the standard output destination for a process.
 *
 *  This class defines how the standard output of a process is managed. It supports 
 *  various output destinations, including files, streams, and pipes. The behavior 
 *  depends on the specified destination:
 *
 *  - If no destination is specified (`PIPE` option), the process writes output 
 *    to `pipe_reader`, which represents the pipe’s read end.
 *  - If a destination is available and a file descriptor can be obtained from it, 
 *    the process output is directly redirected using `::dup2`.
 *  - If a destination is available but does not provide a file descriptor, 
 *    communication between standard output and the destination is emulated using 
 *    a pipe and the `communicate` function.
 *  - If `DEVNULL` is specified, output is discarded by redirecting it to `/dev/null`.
 */
class std_out_t {
public:
    explicit std_out_t(int fd);
    explicit std_out_t(FILE* fp);
    explicit std_out_t(IOOption option);
    explicit std_out_t(std::ostream* stream);
    explicit std_out_t(const std::filesystem::path& file);

    std::shared_ptr<File>        pipe_reader;
    std::shared_ptr<File>        pipe_writer;
    std::shared_ptr<OStreamable> destination;

private:
    static void auto_close(OStreamable* stream) noexcept {
        if (!stream)
            return;
        try {
            stream->close();
        } catch (const std::exception& e) {
            perror("Failed to close the stream");
        }
        delete stream;
    }
};

/** @brief Represents the standard error output destination for a process.
 *
 *  This class defines how the standard error output of a process is managed. 
 *  It supports various output destinations, including files, streams, and pipes. 
 *  The behavior depends on the specified destination:
 *
 *  - If no destination is specified (`PIPE` option), the process writes error 
 *    output to `pipe_reader`, which represents the pipe’s read end.
 *  - If `STDOUT` is specified, error output is redirected to the same 
 *    destination as `std_out_t`.
 *  - If a destination is available and a file descriptor can be obtained from it, 
 *    error output is directly redirected using `::dup2`.
 *  - If a destination is available but does not provide a file descriptor, 
 *    communication between standard error and the destination is emulated using 
 *    a pipe and the `communicate` function.
 *  - If `DEVNULL` is specified, error output is discarded by redirecting it to `/dev/null`.
 */
class std_err_t {
public:
    explicit std_err_t(int fd);
    explicit std_err_t(FILE* fp);
    explicit std_err_t(IOOption option);
    explicit std_err_t(std::ostream* stream);
    explicit std_err_t(const std::filesystem::path& file);

    std::shared_ptr<File>        pipe_reader;
    std::shared_ptr<File>        pipe_writer;
    std::shared_ptr<OStreamable> destination;

    bool is_std_out;

private:
    static void auto_close(OStreamable* stream) noexcept {
        if (!stream)
            return;
        try {
            stream->close();
        } catch (const std::exception& e) {
            perror("Failed to close the stream");
        }
        delete stream;
    }
};

/** @brief Represents a function to be executed before executing a process after a fork.
 *
 *  This class allows the specification of a function that will be executed 
 *  after forking but before executing a new process. It is useful for setting 
 *  up the environment or modifying process attributes before the new process starts.
 */
class preexec_fn_t {
public:
    explicit preexec_fn_t(std::function<void()> fn);
    std::function<void()> preexec_fn;
};

} // namespace types

} // namespace subprocess

#endif