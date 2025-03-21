#ifndef POPEN_H
#define POPEN_H

#include <chrono>
#include <string>
#include <vector>

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>

#include "file.h"
#include "types.h"

namespace subprocess {

struct ConfigRun {
    types::args_t                 args;
    types::bufsize_t              buf_size;
    types::std_in_t               std_in;
    types::std_out_t              std_out;
    types::std_err_t              std_err;
    types::preexec_fn_t           preexec_fn;
    Bytes                         input;   
    std::chrono::duration<double> timeout;
};

struct ConfigProcess {
    explicit ConfigProcess() = default;
    explicit ConfigProcess(const ConfigRun& config)
    : args(config.args),
      bufsize(config.buf_size),
      std_in(config.std_in),
      std_out(config.std_out),
      std_err(config.std_err),
      preexec_fn(config.preexec_fn) {}
     explicit ConfigProcess(ConfigRun&& config)
    : args(std::move(config.args)),
      bufsize(std::move(config.buf_size)),
      std_in(std::move(config.std_in)),
      std_out(std::move(config.std_out)),
      std_err(std::move(config.std_err)),
      preexec_fn(std::move(config.preexec_fn)) {}
       
    types::args_t       args;
    types::bufsize_t    bufsize;
    types::std_in_t     std_in;
    types::std_out_t    std_out;
    types::std_err_t    std_err;
    types::preexec_fn_t preexec_fn;
}; 

class Popen {
public:
    ~Popen()                                 = default;
    Popen(const ConfigProcess& config);
    Popen(ConfigProcess&& config);
    Popen(const Popen& other)                = delete;
    Popen(Popen&& other) noexcept            = delete;

    Popen& operator=(Popen& other)           = delete;
    Popen& operator=(Popen&& other) noexcept = delete;

    std::vector<std::string> args() const;
    ::pid_t                  pid() const;
    ::rusage                 usage() const;
    int                      returncode() const;

    int                      poll();
    int                      wait(std::chrono::duration<double> timeout = std::chrono::duration<double>(0));

    std::pair<Bytes, Bytes>  communicate(const Bytes& input, std::chrono::duration<double> timeout = std::chrono::duration<double>(0));
    void                     send_signal(int signal);
    void                     terminate();
    void                     kill();

    static constexpr int INVALID_RET = (1 << 8);

private:
    void                     set_bufsize(size_t size);
    void                     set_returncode(int status);
    void                     execute();

    std::vector<std::string> args_;
    size_t                   bufsize;
    std::function<void()>    preexec_fn;

    File                     std_in_;
    File                     std_in_handle_;
    File                     std_out_;
    File                     std_out_handle_;
    File                     std_err_;
    File                     std_err_handle_;

    ::pid_t                  pid_;
    ::rusage                 usage_;
    int                      returncode_;
    bool                     is_terminated_;
};

class TimeoutExpired : public std::runtime_error {
public:
    explicit TimeoutExpired(const std::string& msg, std::chrono::duration<double> timeout) 
    : std::runtime_error(msg + " Timed out after " + std::to_string(timeout.count()) +" seconds.") {}
};

struct CompletedProcess {
    std::vector<std::string> args;
    int                      returncode;
    Bytes                    std_out;
    Bytes                    std_err;
    ::rusage                 usage;
};

inline CompletedProcess run(const ConfigRun& config) {
    Popen p = ConfigProcess(config);

    auto [std_out_data, std_err_data] = p.communicate(config.input, config.timeout);

    CompletedProcess result = {
        p.args(),
        p.returncode(),
        std_out_data,
        std_err_data,
        p.usage()
    };

    return result;
}

}

#endif