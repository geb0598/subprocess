#include <thread>

#include <sys/wait.h>

#include "popen.h"

namespace subprocess {

Popen::Popen(const ConfigProcess& config) : pid_(-1), returncode_(INVALID_RET), is_terminated_(false) {
    args_           = config.args.args;
    bufsize         = config.bufsize.bufsize;
    std_in_         = config.std_in.std_in;
    std_in_handle_  = config.std_in.std_in_handle;
    std_out_        = config.std_out.std_out;
    std_out_handle_ = config.std_out.std_out_handle;
    if (config.std_err.is_stdout) {
        std_err_    = config.std_out.std_out;
    } else {
        std_err_    = config.std_err.std_err;
    }
    std_err_handle_ = config.std_err.std_err_handle;
    preexec_fn      = config.preexec_fn.preexec_fn;

    execute();
}

Popen::Popen(ConfigProcess&& config) : pid_(-1), returncode_(INVALID_RET), is_terminated_(false) {
    args_           = std::move(config.args.args);
    bufsize         = std::move(config.bufsize.bufsize);
    std_in_         = std::move(config.std_in.std_in);
    std_in_handle_  = std::move(config.std_in.std_in_handle);
    std_out_        = std::move(config.std_out.std_out);
    std_out_handle_ = std::move(config.std_out.std_out_handle);
    if (config.std_err.is_stdout) {
        std_err_    = std::move(config.std_out.std_out);
    } else {
        std_err_    = std::move(config.std_err.std_err);
    }
    std_err_handle_ = std::move(config.std_err.std_err_handle);
    preexec_fn      = std::move(config.preexec_fn.preexec_fn);

    execute();
}

std::vector<std::string> Popen::args() const {
    return args_;
}

::pid_t Popen::pid() const {
    return pid_;
}

::rusage Popen::usage() const {
    if (!is_terminated_) {
        throw std::runtime_error ( 
            "ERROR::SUBPROCESS:POPEN: Process termination not detected. "
            "Ensure that wait(), poll() or communicate() has been called before accessing resource usage."
        );
    }

    return usage_;
}

int Popen::returncode() const {
    if (!is_terminated_) {
        throw std::runtime_error (
            "ERROR::SUBPROCESS:POPEN: Process termination not detected. "
            "Ensure that wait(), poll() or communicate() has been called before accessing returncode."
        );
    }

    return returncode_;
}

int Popen::poll() {
    if (is_terminated_) {
        return returncode_;
    }

    int status;
    int pid = wait4(pid_, &status, WNOHANG, &usage_);

    if (pid == -1) {
        throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::POPEN: Failed to wait subprocess.");
    } else if (pid == pid_) {
        set_returncode(status);
        is_terminated_ = true;
    } 

    return returncode_;
}

int Popen::wait(std::chrono::duration<double> timeout) {
    if (is_terminated_) {
        return returncode_;
    }

    if (timeout == std::chrono::duration<double>(0)) {
        int status;
        int pid = wait4(pid_, &status, NULL, &usage_);

        if (pid == -1) {
            throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::POPEN: Failed to wait subprocess.");
        } else if (pid == pid_) {
            set_returncode(status);
            is_terminated_ = true;
        }
    } else {
        auto start_time = std::chrono::steady_clock::now();

        while (true) {
            int status;
            int pid = wait4(pid_, &status, WNOHANG, &usage_);

            if (pid == -1) {
                throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::POPEN: Failed to wait subprocess.");
            } else if (pid == pid_) {
                set_returncode(status);
                is_terminated_ = true;
                break;
            }

            auto elapsed_time = std::chrono::steady_clock::now() - start_time;
            if (elapsed_time >= timeout) {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    return returncode_;
}

std::pair<Bytes, Bytes> Popen::communicate(const Bytes& input, std::chrono::duration<double> timeout) {
    if (!input.empty()) {
        std_in_handle_.write(input);
    }

    if (timeout == std::chrono::duration<double>(0)) {
        if (wait(timeout) == INVALID_RET) {
            throw std::runtime_error("ERROR::SUBPROCESS::POPEN: Failed to wait process.");
        }
    } else {
        if (wait(timeout) == INVALID_RET) {
            throw TimeoutExpired("ERROR::SUBPROCESS::POPEN: Failed to communicate with process.", timeout);
        }
    }

    // Close pipe after writing to retrieve result
    std_in_handle_.close();
    fclose(std_in_handle_.filepointer());

    Bytes std_out_data;
    if (std_out_handle_.is_opened()) {
        std_out_data = std_out_handle_.read();
    }

    Bytes std_err_data;
    if (std_err_handle_.is_opened()) {
        std_err_data = std_err_handle_.read();
    }

    return {std_out_data, std_err_data};
}

void Popen::send_signal(int signal) {
    if (!is_terminated_) {
        ::kill(pid_, signal);
    }
}

void Popen::terminate() {
    send_signal(SIGTERM);
}

void Popen::kill() {
    send_signal(SIGKILL);
}

void Popen::set_bufsize(size_t size) {
    int mode;
    if (size == 0) {
        mode = _IONBF;
    } else if (size == 1) {
        mode = _IOLBF;
    } else if (size > 0) {
        mode = _IOFBF;
    } else {
        mode = _IOFBF;
        size = static_cast<size_t>(NULL);
    }

    std::vector<File> files{std_in_, std_in_handle_, std_out_, std_out_handle_, std_err_, std_err_handle_};
    for (auto file : files) {
        if (file.is_opened()) {
            file.set_bufsize(size, mode);
        }
    }
}

void Popen::set_returncode(int status) {
    if (WIFSIGNALED(status)) {
        returncode_ = -WTERMSIG(status);
    } else if (WIFEXITED(status)) {
        returncode_ = WEXITSTATUS(status);
    } else {
        throw std::runtime_error("ERROR::SUBPROCESS::POPEN: Invalid return code detected.");
    }
}

void Popen::execute() {
    pid_ = fork();
    if (pid_ == -1) {
        throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::POPEN: Failed to fork child process.");
    } else if (pid_ == 0) {
        std_in_handle_.close();
        std_out_handle_.close();
        std_err_handle_.close();

        if (std_in_.is_opened()) {
            if (dup2(std_in_.fileno(), STDIN_FILENO) == -1) {
                throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::POPEN: Failed to duplicate input file descriptor.");
            }
        }

        if (std_out_.is_opened()) {
            if (dup2(std_out_.fileno(), STDOUT_FILENO) == -1) {
                throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::POPEN: Failed to duplicate output file descriptor.");
            }
        }

        if (std_err_.is_opened()) {
            if (dup2(std_err_.fileno(), STDERR_FILENO) == -1) {
                throw std::system_error(errno, std::generic_category(), "ERROR::SUBPROCESS::POPEN: Failed to duplicate error file descriptor.");
            }
        }

        std::vector<char*> c_args;
        for (auto& arg : args_) {
            c_args.push_back(arg.data());
        }
        c_args.push_back(nullptr);

        preexec_fn();

        execv(c_args[0], c_args.data());
        perror("ERROR::SUBPROCESS::POPEN: Failed to execute program.");
        exit(EXIT_FAILURE);
    } else {
        std_in_.close();
        std_out_.close();
        std_err_.close();
    }
}

}