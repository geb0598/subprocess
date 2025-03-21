// TODO: header guard

#include <optional>

#include <sys/resource.h>

#include "bytes.h"
#include "streamable.h"
#include "types_new.h"

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
    template<typename Param>
    PopenConfig(Param&& param) { set_value(std::forward(param)); }
    template<typename Param, typename... Params>
    PopenConfig(Param&& param, Params&&... params) : PopenConfig(std::forward(params)...) { 
        set_value(std::forward(param)); 
    }

private:
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

    std::optional<types::args_t>       args       = std::nullopt;
    std::optional<types::bufsize_t>    bufsize    = std::nullopt;
    std::optional<types::std_in_t>     std_in     = std::nullopt; 
    std::optional<types::std_out_t>    std_out    = std::nullopt;
    std::optional<types::std_err_t>    std_err    = std::nullopt;
    std::optional<types::preexec_fn_t> preexec_fn = std::nullopt;
};

class RunConfig : public PopenConfig {};

class Popen {
public:
    ~Popen()                                 = default;
    // TODO: Popen(const PopenConfig& config);
    Popen(PopenConfig&& config);
    Popen(const Popen& other)                = delete;
    Popen(Popen&& other) noexcept            = delete;

    Popen& operator=(const Popen& other)     = delete;
    Popen& operator=(Popen&& other) noexcept = delete;

    std::vector<std::string> args() const;
    std::optional<::pid_t>   pid() const;
    std::optional<::rusage>  usage() const;
    std::optional<int>       returncode() const;
    std::optional<std::shared_ptr<IStream>> std_in() const;
    std::optional<std::shared_ptr<OStream>> std_out() const;
    std::optional<std::shared_ptr<OStream>> std_err() const;

    std::optional<int>       poll();
    std::optional<int>       wait(double timeout);
    std::pair<Bytes, Bytes>  communicate(const Bytes& input, double timeout);
    void                     send_signal(int signal); 
    void                     terminate();
    void                     kill();

private:
    PopenConfig             config_;
    std::optional<::pid_t>  pid_;
    std::optional<::rusage> usage_;
    std::optional<int>      returncode_;
};

}