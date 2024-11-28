#ifndef TYPES_H
#define TYPES_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <filesystem>

#include "readable.h"
#include "writable.h"
#include "utils.h"

namespace subprocess {

namespace type {

enum IOOption {
    NA,
    PIPE,
    STDOUT,
    DEVNULL
};

struct args_t {
    template <typename... Args>
    explicit args_t(Args... args) {
        utils::Concatenator concatenator; 
        value = concatenator.concat(args...);
    } 
    std::string value;
};

struct bufsize_t {
    explicit bufsize_t(size_t bufsize) : value(bufsize) {}
    size_t value = -1;
};

struct executable_t {
    explicit executable_t(const std::string& executable) : value(executable) {}
    std::string value = "/bin/sh";
};

struct preexec_fn_t {
    explicit preexec_fn_t(const std::function<void()> preexec_fn) : value(preexec_fn) {}
    std::function<void()> value = [] {};
};

struct stdin_t {
    explicit stdin_t(int fd) : value(std::make_unique<FileReader>(fd)) {}
    explicit stdin_t(FILE* fp) : value(std::make_unique<FileReader>(fp)) {}
    explicit stdin_t(std::istream& stream) : value(std::make_unique<StreamReader>(stream)) {}
    explicit stdin_t(IOOption opt) : option(opt) {}
    explicit stdin_t(const std::filesystem::path& path) {
        if (std::filesystem::is_empty(path)) {
            throw std::runtime_error(""); // TODO
        }
        // TODO
    }
    std::unique_ptr<IReadable> value;
    IOOption option = NA;
};

struct input_t {

};

struct stdout_t {
    explicit stdout_t(int fd) : value(std::make_unique<FileWriter>(fd)) {}
    explicit stdout_t(FILE* fp) : value(std::make_unique<FileWriter>(fp)) {}
    explicit stdout_t(std::ostream& stream) : value(std::make_unique<StreamWriter>(stream)) {}
    explicit stdout_t(IOOption opt) : option(opt) {}
    explicit stdout_t(const std::filesystem::path& path) {
        if (std::filesystem::is_empty(path)) {
            throw std::runtime_error(""); // TODO
        }
        // TODO
    }
    std::unique_ptr<IWritable> value;
    IOOption option = NA;
};

struct stderr_t {
    explicit stderr_t(int fd) : value(std::make_unique<FileWriter>(fd)) {}
    explicit stderr_t(FILE* fp) : value(std::make_unique<FileWriter>(fp)) {}
    explicit stderr_t(std::ostream& stream) : value(std::make_unique<StreamWriter>(stream)) {}
    explicit stderr_t(IOOption opt) : option(opt) {}
    explicit stderr_t(const std::filesystem::path& path) {
        if (std::filesystem::is_empty(path)) {
            throw std::runtime_error(""); // TODO
        }
        // TODO
    }
    std::unique_ptr<IWritable> value;
    IOOption option = NA;
};

struct capture_output_t {

};

struct shell_t {
    explicit shell_t(bool shell) : value(shell) {}
    bool value = false;
};

struct cwd_t {
    explicit cwd_t(const std::filesystem::path& cwd) : value(cwd) {}
    std::filesystem::path value;
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

}

#endif 