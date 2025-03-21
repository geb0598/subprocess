// TODO: Header guard

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
    explicit bufsize_t(ssize_t bufsize = -1);
    ssize_t bufsize;
};

enum class IOOption { NONE, PIPE, STDOUT, DEVNULL };

/** @brief Represents the standard input source for a process.
 *
 *  This class specifies the source to be used as the standard input for a 
 *  process. It supports different input sources such as files, streams, 
 *  or pipes:
 *
 *  - `PIPE`: Uses a pipe, and the `std_in_handle` represents the pipe’s write end.
 *  - `NONE`: No redirection or use of a pipe for standard input.
 */
class std_in_t {
public:
    explicit std_in_t(int fd);
    explicit std_in_t(FILE* fp);
    explicit std_in_t(IOOption option);
    explicit std_in_t(std::istream* stream);
    explicit std_in_t(const std::filesystem::path& file);

    std::shared_ptr<IStreamable> std_in;
    std::shared_ptr<IStreamable> std_in_handle;

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
 *  This class specifies the destination for the standard output of a 
 *  process. It supports various output options such as files, streams, 
 *  or special options:
 *
 *  - `PIPE`: Uses a pipe, and the `std_out_handle` represents the pipe’s read end.
 *  - `DEVNULL`: Redirects the output to `/dev/null`, effectively discarding it.
 */
class std_out_t {
public:
    explicit std_out_t(int fd);
    explicit std_out_t(FILE* fp);
    explicit std_out_t(IOOption option);
    explicit std_out_t(std::ostream* stream);
    explicit std_out_t(const std::filesystem::path& file);

    std::shared_ptr<OStreamable> std_out;
    std::shared_ptr<OStreamable> std_out_handle;

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
 *  This class specifies the destination for the standard error output 
 *  of a process. It supports various output options such as files, 
 *  streams, or special options:
 *
 *  - `PIPE`: Uses a pipe, and the `std_err_handle` represents the pipe’s read end.
 *  - `STDOUT`: Redirects to the same output destination as `std_out_t`.
 *  - `DEVNULL`: Redirects the error output to `/dev/null`, effectively discarding it.
 */
class std_err_t {
public:
    explicit std_err_t(int fd);
    explicit std_err_t(FILE* fp);
    explicit std_err_t(IOOption option);
    explicit std_err_t(std::ostream* stream);
    explicit std_err_t(const std::filesystem::path& file);

    std::shared_ptr<OStreamable> std_err;
    std::shared_ptr<OStreamable> std_err_handle;
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
 *
 *  The function is provided as a `std::function<void()>` that is invoked 
 *  during the fork process.
 */
class preexec_fn_t {
public:
    explicit preexec_fn_t(std::function<void()> fn = [] {});
    std::function<void()> preexec_fn;
};

} // namespace types

} // namespace subprocess