#include <unistd.h>

#include <fcntl.h>

#include "subprocess/exception.h"
#include "subprocess/types.h"

namespace subprocess {

namespace types {
/* ===================================== bufsize ===================================== */
bufsize_t::bufsize_t(ssize_t bufsize) : bufsize(bufsize) {}

/* ===================================== std_in ===================================== */
std_in_t::std_in_t(int fd)          : pipe_reader(nullptr), pipe_writer(nullptr), source(new File(fd)) {}
std_in_t::std_in_t(FILE* fp)        : pipe_reader(nullptr), pipe_writer(nullptr), source(new File(fp)) {}
std_in_t::std_in_t(IOOption option) : pipe_reader(nullptr), pipe_writer(nullptr), source(nullptr) {
    switch (option) {
        case IOOption::NONE: break;
        case IOOption::PIPE:
            int pipe_fd[2];
            if (::pipe(pipe_fd) == -1) 
                throw OSError(errno, std::generic_category(), "Failed to open pipe");
            pipe_reader = { new File(pipe_fd[0]), auto_close };
            pipe_writer = { new File(pipe_fd[1]), auto_close };
            break;
        default: throw std::invalid_argument("Invalid I/O option for standard input.");
    }
}
std_in_t::std_in_t(std::istream* stream) : pipe_reader(nullptr), pipe_writer(nullptr), source(new IStream(stream)) {
    int pipe_fd[2];
    if (::pipe(pipe_fd) == -1) 
        throw OSError(errno, std::generic_category(), "Failed to open pipe");
    pipe_reader = { new File(pipe_fd[0]), auto_close };
    pipe_writer = { new File(pipe_fd[1]), auto_close };
}
std_in_t::std_in_t(const std::filesystem::path& file) : pipe_reader(nullptr), pipe_writer(nullptr), source(nullptr) {
    if (!std::filesystem::exists(file))
        throw std::invalid_argument("File does not exist: " + file.string());

    FILE* fp = std::fopen(file.c_str(), "r");
    if (fp == nullptr)
        throw OSError(errno, std::generic_category(), "Failed to open file", file);
    
    source = { new File(fp), auto_close };
}

/* ===================================== std_out ===================================== */
std_out_t::std_out_t(int fd)          : pipe_reader(nullptr), pipe_writer(nullptr), destination(new File(fd)) {}
std_out_t::std_out_t(FILE* fp)        : pipe_reader(nullptr), pipe_writer(nullptr), destination(new File(fp)) {}
std_out_t::std_out_t(IOOption option) : pipe_reader(nullptr), pipe_writer(nullptr), destination(nullptr) {
     switch (option) {
        case IOOption::NONE: { break; }
        case IOOption::PIPE: {
            int pipe_fd[2];
            if (::pipe(pipe_fd) == -1)
                throw OSError(errno, std::generic_category(), "Failed to open pipe");
            pipe_reader = { new File(pipe_fd[0]), auto_close };
            pipe_writer = { new File(pipe_fd[1]), auto_close };
            break;
        }
        case IOOption::DEVNULL: {
            int fd = ::open("/dev/null", O_WRONLY);
            if (fd == -1)
                throw OSError(errno, std::generic_category(), "Failed to open dev/null");
            destination = { new File(fd), auto_close };
            break;
        }
        default: { throw std::invalid_argument("Invalid I/O option for standard output."); }
    }
}
std_out_t::std_out_t(std::ostream* stream) : pipe_reader(nullptr), pipe_writer(nullptr), destination(new OStream(stream)) {
    int pipe_fd[2];
    if (::pipe(pipe_fd) == -1)
        throw OSError(errno, std::generic_category(), "Failed to open pipe");
    pipe_reader = { new File(pipe_fd[0]), auto_close };
    pipe_writer = { new File(pipe_fd[1]), auto_close };
}
std_out_t::std_out_t(const std::filesystem::path& file) : pipe_reader(nullptr), pipe_writer(nullptr), destination(nullptr) {
    if (!std::filesystem::exists(file))
        throw std::invalid_argument("File does not exist: " + file.string());

    FILE* fp = std::fopen(file.c_str(), "w");
    if (fp == nullptr)
        throw OSError(errno, std::generic_category(), "Failed to open file", file);
    
    destination = { new File(fp), auto_close };
}

/* ===================================== std_err ===================================== */
std_err_t::std_err_t(int fd)          : pipe_reader(nullptr), pipe_writer(nullptr), destination(new File(fd)), is_std_out(false) {}
std_err_t::std_err_t(FILE* fp)        : pipe_reader(nullptr), pipe_writer(nullptr), destination(new File(fp)), is_std_out(false) {}
std_err_t::std_err_t(IOOption option) : pipe_reader(nullptr), pipe_writer(nullptr), destination(nullptr), is_std_out(false) {
     switch (option) {
        case IOOption::NONE: { break; }
        case IOOption::PIPE: {
            int pipe_fd[2];
            if (::pipe(pipe_fd) == -1)
                throw OSError(errno, std::generic_category(), "Failed to open pipe");
            pipe_reader = { new File(pipe_fd[0]), auto_close };
            pipe_writer = { new File(pipe_fd[1]), auto_close };
            break;
        }
        case IOOption::STDOUT: {
            is_std_out = true;
            break;
        }
        case IOOption::DEVNULL: {
            int fd = ::open("/dev/null", O_WRONLY);
            if (fd == -1)
                throw OSError(errno, std::generic_category(), "Failed to open dev/null");
            destination = { new File(fd), auto_close };
            break;
        }
        default: { throw std::invalid_argument("Invalid I/O option for standard error."); }
    }
}
std_err_t::std_err_t(std::ostream* stream) : pipe_reader(nullptr), pipe_writer(nullptr), destination(new OStream(stream)), is_std_out(false) {}
std_err_t::std_err_t(const std::filesystem::path& file) : pipe_reader(nullptr), pipe_writer(nullptr), destination(nullptr), is_std_out(false) {
    if (!std::filesystem::exists(file))
        throw std::invalid_argument("File does not exist: " + file.string());

    FILE* fp = std::fopen(file.c_str(), "w");
    if (fp == nullptr)
        throw OSError(errno, std::generic_category(), "Failed to open file", file);
    
    destination = { new File(fp), auto_close };
}

/* ===================================== preexec_fn ===================================== */
preexec_fn_t::preexec_fn_t(std::function<void()> preexec_fn) : preexec_fn(preexec_fn) {}

} // namespace types

} // namespace subprocess