#include <sys/wait.h>

#include "subprocess/exception.h"
#include "subprocess/popen.h"

namespace subprocess {

void PopenConfig::set_value(const types::args_t& args)             { this->args = args; }
void PopenConfig::set_value(types::args_t&& args)                  { this->args = std::move(args); }
void PopenConfig::set_value(const types::bufsize_t& bufsize)       { this->bufsize = bufsize; }
void PopenConfig::set_value(types::bufsize_t&& bufsize)            { this->bufsize = std::move(bufsize); }
void PopenConfig::set_value(const types::std_in_t& std_in)         { this->std_in = std_in; }
void PopenConfig::set_value(types::std_in_t&& std_in)              { this->std_in = std::move(std_in); }
void PopenConfig::set_value(const types::std_out_t& std_out)       { this->std_out = std_out; }
void PopenConfig::set_value(types::std_out_t&& std_out)            { this->std_out = std::move(std_out); }
void PopenConfig::set_value(const types::std_err_t& std_err)       { this->std_err = std_err; }
void PopenConfig::set_value(types::std_err_t&& std_err)            { this->std_err = std::move(std_err); }
void PopenConfig::set_value(const types::preexec_fn_t& preexec_fn) { this->preexec_fn = preexec_fn; }
void PopenConfig::set_value(types::preexec_fn_t&& preexec_fn)      { this->preexec_fn = std::move(preexec_fn); }

void PopenConfig::validate() {
    if (!args)       throw std::invalid_argument("Missing required 'args' argument.");
    if (!bufsize)    throw std::invalid_argument("Missing required 'bufsize' argument.");
    if (!std_in)     throw std::invalid_argument("Missing required 'std_in' argument.");
    if (!std_out)    throw std::invalid_argument("Missing required 'std_out' argument.");
    if (!std_err)    throw std::invalid_argument("Missing required 'std_err' argument.");
    if (!preexec_fn) throw std::invalid_argument("Missing required 'preexec_fn' argument.");
}

/* ===================================== Popen ===================================== */

Popen::Popen(PopenConfig&& config) : config_(std::move(config)), pid_(-1), usage_(std::nullopt), returncode_(std::nullopt) {
    /** Throws a std::invalid_argument exception when required argument is missing. */
    config_.validate();

    /** Alias references for optional configuration values. */
    auto& args       = config_.args.value();
    auto& std_in     = config_.std_in.value();
    auto& std_out    = config_.std_out.value();
    auto& std_err    = config_.std_err.value();
    auto& bufsize    = config_.bufsize.value();
    auto& preexec_fn = config_.preexec_fn.value();

    /** Pipe handles for the parent process. */
    File* parent_fps[3] = { 
        std_in.pipe_writer.get(), 
        std_out.pipe_reader.get(), 
        std_err.pipe_reader.get() 
    };
    for (auto fp : parent_fps) {
        if (fp && fp->is_opened()) {
            fp->set_bufsize(bufsize.bufsize);
        }
    }

    if (std_err.is_std_out)
        std_err.pipe_writer = std_out.pipe_writer;

    /** Pipe handles for the child process. */
    File* child_fps[3] = {
        std_in.pipe_reader.get(), 
        std_out.pipe_writer.get(), 
        std_err.pipe_writer.get() 
    };

    /** Streams source and destinations for stdin, stdout and stderr with matching file descriptors. */
    std::pair<Streamable*, int> streams[3] = {
        { static_cast<Streamable*>(std_in.source.get()),       STDIN_FILENO  },
        { static_cast<Streamable*>(std_out.destination.get()), STDOUT_FILENO },
        { static_cast<Streamable*>(std_err.destination.get()), STDERR_FILENO }
    };

    pid_ = ::fork();
    if (pid_ == -1) {
        throw std::runtime_error("Failed to fork a process.");
    } else if (pid_ == 0) {
        for (auto fp : parent_fps) {
            if (fp) fp->close();
        }
        /** If the stream is of a type that provides a valid file descriptor,  
         *  use dup2 to directly connect the child process's stdin, stdout, or stderr. */
        for (int i = 0; i < 3; ++i) {
            auto [stream, fd] = streams[i];
            auto fp           = child_fps[i];
            if (stream && stream->fileno() != -1) {
                if (::dup2(stream->fileno(), fd) == -1) {
                    ::perror("Failed to duplicate file descriptor.");
                    ::_exit(EXIT_FAILURE);
                }
            } else if (fp && fp->fileno() != -1) {
                if (::dup2(fp->fileno(), fd) == -1) {
                    ::perror("Failed to duplicate file descriptor.");
                    ::_exit(EXIT_FAILURE);
                }
            }
        }

        // TODO: clo_exec
        for (auto fp : child_fps) {
            if (fp) fp->close();
        }
        for (auto [stream, fd] : streams) {
            if (stream) stream->close();
        }

        preexec_fn.preexec_fn();

        std::vector<char*> c_args;
        for (auto& arg : args.args) 
            c_args.push_back(arg.data());
        c_args.push_back(nullptr);
        ::execv(c_args[0], c_args.data());
        ::perror("Failed to execute a program");
        ::_exit(EXIT_FAILURE);
    } else {
        for (auto fp : child_fps) {
            if (fp) fp->close();
        }
        /** If a source or destination is specified, start communication with a pipe connected 
         * to child process through a thread, simulating the behavior of dup2. */
        for (int i = 0; i < 3; ++i) {
            if (streams[i].first && parent_fps[i]) {
                IStreamable* istream;
                OStreamable* ostream;
                if (i == 0) {
                    istream = dynamic_cast<IStreamable*>(streams[i].first);
                    ostream = parent_fps[i];
               } else {
                    istream = parent_fps[i];
                    ostream = dynamic_cast<OStreamable*>(streams[i].first);
                }
                comm_results[i] = communicate_async(*istream, *ostream, true); 
            } 
        }
    }
}

std::vector<std::string> Popen::args() const {
    if (!config_.args.has_value())
        throw std::runtime_error("Missing required 'args' argument.");
    return config_.args.value().args;
}
::pid_t                 Popen::pid() const        { return pid_;        }
std::optional<::rusage> Popen::usage() const      { return usage_;      }
std::optional<int>      Popen::returncode() const { return returncode_; }

std::optional<std::shared_ptr<OStreamable>> Popen::std_in() {
    if (!config_.std_in.has_value())
        throw std::runtime_error("Missing required 'std_in' argument.");
    auto& std_in = config_.std_in.value();
    if (std_in.pipe_writer && std_in.pipe_writer->is_opened() && !(std_in.source && std_in.source->is_opened()))
        return std_in.pipe_writer;
    else
        return std::nullopt;
}
std::optional<std::shared_ptr<IStreamable>> Popen::std_out() {
    if (!config_.std_out.has_value())
        throw std::runtime_error("Missing required 'std_out' argument."); 
    auto& std_out = config_.std_out.value();
    if (std_out.pipe_reader && std_out.pipe_reader->is_opened() && !(std_out.destination && std_out.destination->is_opened()))
        return std_out.pipe_reader;
    else
        return std::nullopt;
}
std::optional<std::shared_ptr<IStreamable>> Popen::std_err() {
    if (!config_.std_err.has_value())
        throw std::runtime_error("Missing required 'std_err' argument."); 
    auto& std_err = config_.std_err.value();
    if (std_err.pipe_reader && std_err.pipe_reader->is_opened() && !(std_err.destination && std_err.destination->is_opened()))
        return std_err.pipe_reader;
    else
        return std::nullopt;
}

std::optional<int> Popen::poll() {
    if (returncode())
        return returncode();

    int status;
    ::rusage usage;
    int pid = ::wait4(pid_, &status, WNOHANG, &usage);

    if (pid == -1) {
        throw OSError(errno, std::generic_category(), "Failed to wait process");
    } else if (pid == pid_) {
        /** Wait until the entire asynchronous communication is complete. */
        comm_wait(); 
        set_returncode(status);
        usage_ = usage;
    }

    return returncode();
}
std::optional<int> Popen::wait(double timeout) {
    if (timeout < 0) {
        while (!poll())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return returncode();
    } else {
        auto start_time = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start_time < std::chrono::duration<double>(timeout)) {
            if (poll())
                return returncode();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        throw TimeoutExpired("Failed to wait", std::chrono::steady_clock::now() - start_time);
    }
}

std::pair<
    std::optional<Bytes>, 
    std::optional<Bytes>> Popen::communicate(const Bytes& input, double timeout) {
    auto& std_in  = config_.std_in.value();
    auto& std_out = config_.std_out.value();
    auto& std_err = config_.std_err.value();
    if (std_in.pipe_writer && std_in.pipe_writer->is_opened())
        std_in.pipe_writer->write(input, input.size()); 
    else
        throw std::runtime_error("Pipe is not opened.");

    if (std_in.pipe_writer)
        std_in.pipe_writer->close();

    wait(timeout);

    std::optional<Bytes> std_out_data;
    if (std_out.pipe_reader && std_out.pipe_reader->is_opened()) {
        std_out_data = std_out.pipe_reader->read_all();
        std_out.pipe_reader->close();
    }

    std::optional<Bytes> std_err_data;
    if (std_err.pipe_reader && std_err.pipe_reader->is_opened()) {
        std_err_data = std_err.pipe_reader->read_all();
        std_err.pipe_reader->close();
    }
    
    return {std_out_data, std_err_data};
}

void Popen::send_signal(int signal) {
    if (!returncode())
        ::kill(pid_, signal);
}
void Popen::terminate() { send_signal(SIGTERM); }
void Popen::kill()      { send_signal(SIGKILL); }

void Popen::comm_wait() {
    for (auto& comm_result : comm_results) {
        if (comm_result.valid())
            comm_result.wait();
    }
}

void Popen::set_returncode(int status) {
    if (WIFSIGNALED(status)) 
        returncode_ = -WTERMSIG(status);
    else if (WIFEXITED(status))
        returncode_ = WEXITSTATUS(status);
    else
        throw std::runtime_error("Invalid return code detected.");
}

} // namespace subprocess