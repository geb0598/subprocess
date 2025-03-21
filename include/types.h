#ifndef TYPES_H
#define TYPES_H

#include <functional>

#include "popen.h"

namespace subprocess {

namespace types {

struct args_t {
    explicit args_t() = default;

    template<typename... Params>
    explicit args_t(Params&&... params) : args(concat(std::forward<Params>(params)...)) {}

    template<typename... Params>
    std::vector<std::string> concat(Params&&... params) { return {std::forward<Params>(params)...}; }

    std::vector<std::string> args;
};

struct bufsize_t {
    explicit bufsize_t() : bufsize(-1) {}
    explicit bufsize_t(size_t bufsize) : bufsize(bufsize) {}

    size_t bufsize;
};

enum class IOOption {
    NONE,
    PIPE,
    STDOUT,
    DEVNULL
};

struct std_in_t {
    explicit std_in_t() = default;
    explicit std_in_t(FILE* fp) : std_in(fp) {}
    explicit std_in_t(int fd) : std_in(fd) {}
    explicit std_in_t(IOOption option) {
        switch (option) {
            case IOOption::NONE:
                break;
            case IOOption::PIPE:
                int pipe_fd[2];
                if (pipe(pipe_fd) == -1) {
                    throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS: Failed to open pipe.");
                }
                std_in.open(pipe_fd[0], true);
                std_in_handle.open(pipe_fd[1], true);
                break;
            default:
                throw std::runtime_error("ERROR::SUBPROCESS: Invalid I/O option.");
        }
    }
    explicit std_in_t(const std::filesystem::path& file) {
        if (!std::filesystem::exists(file)) {
            throw std::runtime_error("ERROR::SUBPROCESS: File '" + file.string() + "' not exists.");
        }

        FILE* fp = fopen(file.c_str(), filemode_to_string(FileMode::READ_BYTE).c_str());
        if (fp == nullptr) {
            throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS: Failed to open file '" + file.string() + "'.");
        }
        std_in.open(fp, true);
    }

    File std_in;
    File std_in_handle;
};

struct std_out_t {
    explicit std_out_t() = default;
    explicit std_out_t(FILE* fp) : std_out(fp) {}
    explicit std_out_t(int fd) : std_out(fd) {}
    explicit std_out_t(IOOption option) {
        switch (option) {
            case IOOption::NONE: {
                break;
            }
            case IOOption::PIPE: {
                int pipe_fd[2];
                if (pipe(pipe_fd) == -1) {
                    throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS: Failed to open pipe.");
                }
                std_out.open(pipe_fd[1], true);
                std_out_handle.open(pipe_fd[0], true);
                break;
            }
            case IOOption::DEVNULL: {
                FILE* fp = fopen("/dev/null", filemode_to_string(FileMode::WRITE_BYTE).c_str());
                if (fp == nullptr) {
                    throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS: Failed to open /dev/null.");
                }
                std_out.open(fp, true);
                break;
            }
            default: {
                throw std::runtime_error("ERROR::SUBPROCESS: Invalid I/O option.");
            }
        }
    }
    explicit std_out_t(const std::filesystem::path& file) {
        if (!std::filesystem::exists(file)) {
            throw std::runtime_error("ERROR::SUBPROCESS: File '" + file.string() + "' not exists.");
        }

        FILE* fp = fopen(file.c_str(), filemode_to_string(FileMode::WRITE_BYTE).c_str());
        if (fp == nullptr) {
            throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS: Failed to open file '" + file.string() + "'.");
        }
        std_out.open(fp, true);
    }

    File std_out;
    File std_out_handle;
};

struct std_err_t {
    explicit std_err_t() = default;
    explicit std_err_t(FILE* fp) : std_err(fp) {}
    explicit std_err_t(int fd) : std_err(fd) {}
    explicit std_err_t(IOOption option) {
        switch (option) {
            case IOOption::NONE: {
                break;
            }
            case IOOption::PIPE: {
                int pipe_fd[2];
                if (pipe(pipe_fd) == -1) {
                    throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS: Failed to open pipe.");
                }
                std_err.open(pipe_fd[1], true);
                std_err_handle.open(pipe_fd[0], true);
                break;
            }
            case IOOption::DEVNULL: {
                FILE* fp = fopen("/dev/null", filemode_to_string(FileMode::WRITE_BYTE).c_str());
                if (fp == nullptr) {
                    throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS: Failed to open /dev/null.");
                }
                std_err.open(fp, true);
                break;
            }
            case IOOption::STDOUT: {
                is_stdout = true;
                break;
            }
            default: {
                throw std::runtime_error("ERROR::SUBPROCESS: Invalid I/O option.");
            }
        }
    }
    explicit std_err_t(const std::filesystem::path& file) {
        if (!std::filesystem::exists(file)) {
            throw std::runtime_error("ERROR::SUBPROCESS: File '" + file.string() + "' not exists.");
        }

        FILE* fp = fopen(file.c_str(), filemode_to_string(FileMode::WRITE_BYTE).c_str());
        if (fp == nullptr) {
            throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS: Failed to open file '" + file.string() + "'.");
        }
        std_err.open(fp, true);
    }

    File std_err;
    File std_err_handle;
    bool is_stdout = false;
};

struct preexec_fn_t {
    explicit preexec_fn_t() : preexec_fn([] {}) {}
    explicit preexec_fn_t(std::function<void()> fn) : preexec_fn(fn) {}

    std::function<void()> preexec_fn;
};

}

}

#endif