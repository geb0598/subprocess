#ifndef POPEN_H
#define POPEN_H

#include <optional>

#include <sys/resource.h>

#include "bytes.h"
#include "streamable.h"
#include "types.h"

namespace subprocess {

/** @brief Configuration class for process spawning.
 *
 *  Supports a flexible constructor that allows parameters in any order, similar to Python.
 *  However, the internal state prevents using the same configuration for multiple process spawns.
 * 
 *  @note Ensure that not to use the same configuration for multiple process spawns due to its internal state.
 *        The recommended way to use this class is by passing a temporary object as an argument to Popen constructor.
 *
 *  @todo Implement reset() method to allow reusing the configuration.
 */
class PopenConfig {
public:
    template<typename... Params>
    PopenConfig(Params&&... params) { set_value(std::forward<Params>(params)...); }
    template<typename Param, typename... Params>
    void set_value(Param&& param, Params&&... params) {
        set_value(std::forward<Param>(param));
        set_value(std::forward<Params>(params)...);
    }
    void set_value(const types::args_t& args);       
    void set_value(types::args_t&& args);            
    void set_value(const types::bufsize_t& bufsize); 
    void set_value(types::bufsize_t&& bufsize);      
    void set_value(const types::std_in_t& std_in);
    void set_value(types::std_in_t&& std_in);
    void set_value(const types::std_out_t& std_out);
    void set_value(types::std_out_t&& std_out);
    void set_value(const types::std_err_t& std_err);
    void set_value(types::std_err_t&& std_err);
    void set_value(const types::preexec_fn_t& preexec_fn);
    void set_value(types::preexec_fn_t&& preexec_fn);

    void validate();

    /** Variables initialized with std::nullopt must be explicitly provided in the constructor.  
     *  Others have default values specified here. */
    std::optional<types::args_t>       args       = std::nullopt;
    std::optional<types::bufsize_t>    bufsize    = types::bufsize_t(-1);
    std::optional<types::std_in_t>     std_in     = types::std_in_t(types::IOOption::NONE); 
    std::optional<types::std_out_t>    std_out    = types::std_out_t(types::IOOption::NONE);
    std::optional<types::std_err_t>    std_err    = types::std_err_t(types::IOOption::NONE);
    std::optional<types::preexec_fn_t> preexec_fn = types::preexec_fn_t([] {});
};

// TODO
class RunConfig : public PopenConfig {};

class Popen {
public:
    ~Popen()                                 = default;
    Popen(PopenConfig&& config);
    Popen(const Popen& other)                = delete;
    Popen(Popen&& other) noexcept            = delete;

    Popen& operator=(const Popen& other)     = delete;
    Popen& operator=(Popen&& other) noexcept = delete;

    std::vector<std::string> args() const;
    ::pid_t                  pid() const;
    /** usage is set by a call to the poll(), wait(), or communicate() methods if they detect that the process has terminated. */
    std::optional<::rusage>  usage() const;
    /** returncode is set by a call to the poll(), wait(), or communicate() methods if they detect that the process has terminated. */
    std::optional<int>       returncode() const;

    /** If the stdin was set to PIPE, this returns a writable stream. Otherwise, this returns std::nullopt */
    std::optional<std::shared_ptr<OStreamable>> std_in();
    /** If the stdout was set to PIPE, this returns a readable stream. Otherwise, this returns std::nullopt */
    std::optional<std::shared_ptr<IStreamable>> std_out();
    /** If the stderr was set to PIPE, this returns a readable stream. Otherwise, this returns std::nullopt */
    std::optional<std::shared_ptr<IStreamable>> std_err();

    /** @brief Checks if the process has exited.
     *
     *  If the process has terminated, the function returns an optional containing 
     *  the exit code and sets the relevant state. Otherwise, it returns an empty optional.
     * 
     *  @note This function also waits for the entire asynchronous communication 
     *        to complete when the process is terminated.
     */
    std::optional<int>        poll();
    /** @brief Waits for the process to exit.
     *
     *  Blocks until the process terminates or the specified timeout elapses.
     *  If the timeout is negative, it waits indefinitely. If the timeout expires
     *  before the process exits, an exception may be thrown.
     *
     *  @param timeout Maximum time to wait in seconds (default: -1, meaning wait indefinitely).
     *  @return std::optional<int> Exit code of the process.
     *  @throws TimeoutExpired If the process does not terminate within the specified timeout.
     */
    std::optional<int>        wait(double timeout = -1);
    /** @brief Exchanges data with the process via stdin, stdout, and stderr.
     *
     *  Sends input data to the process's stdin and reads from stdout and stderr 
     *  until EOF. Waits for the process to exit and returns the collected output.
     * 
     *  This function may block indefinitely if the process does not read from 
     *  stdin or if stdout/stderr buffers are full and not being drained.
     *
     *  @param input Data to send to the process (empty if no input is provided).
     *  @param timeout Maximum time to wait in seconds (default: indefinite).
     *  @return std::pair<std::optional<Bytes>, std::optional<Bytes>> 
     *         A pair containing stdout and stderr data, respectively.
     *  @throws TimeoutExpired If the process does not terminate within the timeout.
     */
    std::pair<
        std::optional<Bytes>, 
        std::optional<Bytes>> communicate(const Bytes& input, double timeout = -1);
    void                      send_signal(int signal); 
    void                      terminate();
    void                      kill();

private:
    void                      comm_wait();
    void                      set_returncode(int status);

    PopenConfig                   config_;
    ::pid_t                       pid_;
    std::optional<::rusage>       usage_;
    std::optional<int>            returncode_;
    std::future<Bytes::size_type> comm_results[3];
};

} // namespace subprocess

#endif