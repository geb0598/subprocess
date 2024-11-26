#ifndef TYPES_H
#define TYPES_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <filesystem>

#include "readable.h"
#include "writable.h"

namespace subprocess {

enum IOOption {
    PIPE,
    STDOUT,
    DEVNULL
};

struct args_t {
    template <typename... Args>
    explicit args_t(Args... args) {
        std::ostringstream oss;
        ((oss << args << " "), ...);
        std::string buf = oss.str();
        if (!buf.empty()) {
            buf.pop_back();
        }
        data = buf;
    }
    std::string data;
};

struct bufsize_t {
    explicit bufsize_t(size_t bufsize) : data(bufsize) {}
    size_t data = -1;
};

struct executable_t {
    explicit executable_t(const std::string& executable) : data(executable) {}
    std::string data = "/bin/sh";
};

struct preexec_fn_t {
    explicit preexec_fn_t(const std::function<void()> preexec_fn) : data(preexec_fn) {}
    std::function<void()> data = [] {};
};

struct stdin_t {
    explicit stdin_t(int fd) : data(std::make_unique<FileReader>(fd)) {}
    explicit stdin_t(FILE* fp) : data(std::make_unique<FileReader>(fp)) {}
    explicit stdin_t(std::istream& stream) : data(std::make_unique<StreamReader>(stream)) {}
    explicit stdin_t(const std::filesystem::path& path) {
        if (std::filesystem::is_empty(path)) {
            throw std::runtime_error(""); // TODO
        }
        // TODO
    }
    explicit stdin_t(IOOption option) {
        // TODO switch-case
        if (option == PIPE) {
            // TODO
        } else if (option == DEVNULL) {
            stdin_t("/dev/null");
        } else {
            throw std::runtime_error(""); // TODO
        }
    }
    std::unique_ptr<IReadable> data;
};

struct input_t {

};

struct stdout_t {
    explicit stdout_t(int fd) : data(std::make_unique<FileWriter>(fd)) {}
    explicit stdout_t(FILE* fp) : data(std::make_unique<FileWriter>(fp)) {}
    explicit stdout_t(std::ostream& stream) : data(std::make_unique<StreamWriter>(stream)) {}
    explicit stdout_t(const std::filesystem::path& path); // TODO
    explicit stdout_t(IOOption option) {
        if (option == PIPE) {

        } else if (option == DEVNULL) {

        } else {

        }
    }
    std::unique_ptr<IWritable> data;
};

struct stderr_t {
    explicit stderr_t(int fd) : data(std::make_unique<FileWriter>(fd)) {}
    explicit stderr_t(FILE* fp) : data(std::make_unique<FileWriter>(fp)) {}
    explicit stderr_t(std::ostream& stream) : data(std::make_unique<StreamWriter>(stream)) {}
    explicit stderr_t(const std::filesystem::path& path); // TODO
    explicit stderr_t(IOOption option) {
        if (option == PIPE) {

        } else if (option == DEVNULL) {

        } else if (option == STDOUT) {

        } else {

        }
    }
    std::unique_ptr<IWritable> data;
};

struct capture_output_t {

};

struct shell_t {
    explicit shell_t(bool shell) : data(shell) {}
    bool data = false;
};

struct cwd_t {
    explicit cwd_t(const std::filesystem::path& cwd) : data(cwd) {}
    std::filesystem::path data;
};

struct timeout_t {
    
};

struct check_t {

};

struct encoding {

};

struct errors {

};

struct text {

};

struct env {

};

struct universal_newlines {

};

struct startupinfo {

};

struct creationflags {

};

struct resotre_signals {

};

struct start_new_session {

};

struct pass_fds {

};

struct group {

};

struct extra_groups {

};

struct memory_limit {

};

}

#endif 