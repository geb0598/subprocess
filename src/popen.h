#ifndef POPEN_H
#define POPEN_H

#include <utility>
#include <memory>

#include "bytes.h"
#include "types.h"
#include "readable.h"
#include "writable.h"

namespace subprocess {

class PopenBuilder {
public:
    Popen* build();
    void set_args(type::args_t&& param);
    void set_bufsize(type::bufsize_t&& param);
    void set_executable(type::executable_t&& param);
    void set_preexec_fn(type::preexec_fn_t&& param);
    void set_stdin(type::stdin_t&& param);
    void set_stdout(type::stdout_t&& param);
    void set_stderr(type::stderr_t&& param);

private:
    type::args_t        args_;
    type::bufsize_t     bufsize_;
    type::executable_t  executable_;
    type::preexec_fn_t  preexec_fn_;
    type::stdin_t       stdin_;
    type::stdout_t      stdout_;
    type::stderr_t      stderr_;
};

class Popen {
public:
    ~Popen();

    Popen();
    template <typename... Args>
    Popen(Args&& ...args);

    int poll();
    int wait(int timeout = 0);
    std::pair<Bytes, Bytes> communicate(const std::string& message, int timeout = 0);
    void send_signal(int sig);
    void terminate();
    void kill();

    // args: vector of strings
    // stdin:  
    // stdout: 
    // stderr: 
    std::string args() const;
    FILE* stdin() const;
    FILE* stdout() const;
    FILE* stderr() const;
    pid_t pid() const;
    int returncode() const;

private:
    template<typename T, typename... Args>
    void set_args(PopenBuilder& builder, T&& arg, Args&& ...args);
    void set_args(PopenBuilder& builder);


    std::unique_ptr<IReadable> input_;
    std::unique_ptr<IWritable> output_;
    std::unique_ptr<IWritable> error_;

    std::unique_ptr<PipeReader> pipe_in_;
    std::unique_ptr<PipeWriter> pipe_out_;
    std::unique_ptr<PipeWriter> pipe_err_;
};

}

#endif