#include <unistd.h>

#include <fcntl.h>

#include "exception.h"
#include "types_new.h"

namespace subprocess {

namespace types {
/* ===================================== bufsize ===================================== */
bufsize_t::bufsize_t(ssize_t bufsize) : bufsize(bufsize) {}

/* ===================================== std_in ===================================== */
std_in_t::std_in_t(int fd) : std_in(new File(fd)), std_in_handle(nullptr) {}
std_in_t::std_in_t(FILE* fp) : std_in(new File(fp)), std_in_handle(nullptr) {}
std_in_t::std_in_t(IOOption option) : std_in(nullptr), std_in_handle(nullptr) {
    switch (option) {
        case IOOption::NONE: break;
        case IOOption::PIPE:
            int pipe_fd[2];
            if (::pipe(pipe_fd) == -1)
                throw OSError(errno, std::generic_category(), "Failed to open pipe");
            std_in        = { new File(pipe_fd[0]), auto_close };
            std_in_handle = { new File(pipe_fd[1]), auto_close };
            break;
        default: throw std::invalid_argument("Invalid I/O option for standard input.");
    }
}
std_in_t::std_in_t(std::istream* stream) : std_in(new IStream(stream)), std_in_handle(nullptr) {}
std_in_t::std_in_t(const std::filesystem::path& file) : std_in(nullptr), std_in_handle(nullptr) {
    if (!std::filesystem::exists(file))
        throw std::invalid_argument("File does not exist: " + file.string());

    FILE* fp = std::fopen(file.c_str(), "r");
    if (fp == nullptr)
        throw OSError(errno, std::generic_category(), "Failed to open file", file);
    
    std_in = { new File(fp), auto_close };
}

/* ===================================== std_out ===================================== */
std_out_t::std_out_t(int fd) : std_out(new File(fd)), std_out_handle(nullptr) {}
std_out_t::std_out_t(FILE* fp) : std_out(new File(fp)), std_out_handle(nullptr) {}
std_out_t::std_out_t(IOOption option) : std_out(nullptr), std_out_handle(nullptr) {
     switch (option) {
        case IOOption::NONE: break;
        case IOOption::PIPE:
            int pipe_fd[2];
            if (::pipe(pipe_fd) == -1)
                throw OSError(errno, std::generic_category(), "Failed to open pipe");
            std_out        = { new File(pipe_fd[1]), auto_close };
            std_out_handle = { new File(pipe_fd[0]), auto_close };
            break;
        case IOOption::DEVNULL:
            int fd = ::open("/dev/null", O_WRONLY);
            if (fd == -1)
                throw OSError(errno, std::generic_category(), "Failed to open dev/null");
            std_out = { new File(fd), auto_close };
            break;
        default: throw std::invalid_argument("Invalid I/O option for standard output.");
    }
}
std_out_t::std_out_t(std::ostream* stream) : std_out(new OStream(stream)), std_out_handle(nullptr) {}
std_out_t::std_out_t(const std::filesystem::path& file) : std_out(nullptr), std_out_handle(nullptr) {
    if (!std::filesystem::exists(file))
        throw std::invalid_argument("File does not exist: " + file.string());

    FILE* fp = std::fopen(file.c_str(), "w");
    if (fp == nullptr)
        throw OSError(errno, std::generic_category(), "Failed to open file", file);
    
    std_out = { new File(fp), auto_close };
}

/* ===================================== std_err ===================================== */
std_err_t::std_err_t(int fd) : std_err(new File(fd)), std_err_handle(nullptr), is_std_out(false) {}
std_err_t::std_err_t(FILE* fp) : std_err(new File(fp)), std_err_handle(nullptr), is_std_out(false) {}
std_err_t::std_err_t(IOOption option) : std_err(nullptr), std_err_handle(nullptr), is_std_out(false) {
     switch (option) {
        case IOOption::NONE: break;
        case IOOption::PIPE:
            int pipe_fd[2];
            if (::pipe(pipe_fd) == -1)
                throw OSError(errno, std::generic_category(), "Failed to open pipe");
            std_err        = { new File(pipe_fd[1]), auto_close };
            std_err_handle = { new File(pipe_fd[0]), auto_close };
            break;
        case IOOption::STDOUT:
            is_std_out = true;
            break;
        case IOOption::DEVNULL:
            int fd = ::open("/dev/null", O_WRONLY);
            if (fd == -1)
                throw OSError(errno, std::generic_category(), "Failed to open dev/null");
            std_err = { new File(fd), auto_close };
            break;
        default: throw std::invalid_argument("Invalid I/O option for standard error.");
    }
}
std_err_t::std_err_t(std::ostream* stream) : std_err(new OStream(stream), std::default_delete<OStreamable>()), std_err_handle(nullptr), is_std_out(false) {}
std_err_t::std_err_t(const std::filesystem::path& file) : std_err(nullptr), std_err_handle(nullptr), is_std_out(false) {
    if (!std::filesystem::exists(file))
        throw std::invalid_argument("File does not exist: " + file.string());

    FILE* fp = std::fopen(file.c_str(), "w");
    if (fp == nullptr)
        throw OSError(errno, std::generic_category(), "Failed to open file", file);
    
    std_err = { new File(fp), auto_close };
}

/* ===================================== preexec_fn ===================================== */
preexec_fn_t::preexec_fn_t(std::function<void()> preexec_fn) : preexec_fn(preexec_fn) {}

} // namespace types

} // namespace subprocess