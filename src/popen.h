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


private:
    args_t args_;
    bufsize_t bufsize_;
    executable_t executable_;
    stdin_t stdin_;
    stdout_t stdout_;
    stderr_t stderr_;
    input_t input_;
    capture_output_t capture_output_;
    shell_t shell_;
    cwd_t cwd_;
    timeout_t timeout_;

};

class Popen {
public:
    ~Popen();

    template <typename... Args>
    Popen(const std::string& args, Args... opt_args);

    template <typename... Args>
    Popen(const std::vector<std::string>& args, Args... opt_args);

    template <typename... Args>
    Popen(const std::initializer_list<std::string>& args, Args... opt_args);

    int poll();
    int wait(int timeout = 0);
    std::pair<Bytes, Bytes> communicate(stdin input, int timeout = 0);
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
    template <typename T, typename... Args>
    void parse_args(PopenBuilder& builder, T arg, Args... opt_args);

    void parse_args(PopenBuilder& builder);

    std::unique_ptr<IReadable> input_;
    std::unique_ptr<IWritable> output_;
    std::unique_ptr<IWritable> error_;

    std::unique_ptr<IReadable> pipe_in_;
    std::unique_ptr<IWritable> pipe_out_;
    std::unique_ptr<IWritable> pipe_err_;
};

}

#endif