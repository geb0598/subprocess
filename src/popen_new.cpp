#include "popen_new.h"

namespace subprocess {

void PopenConfig::set_value(types::args_t&& args) {
    if (this->args.has_value())
        throw std::invalid_argument("Duplicated argument detected: args");
    this->args = std::move(args);
}
void PopenConfig::set_value(const types::bufsize_t& bufsize) {
    if (this->bufsize.has_value())
        throw std::invalid_argument("Duplicated argument detected: bufsize");
    this->bufsize = bufsize;
}
void PopenConfig::set_value(types::bufsize_t&& bufsize) {
    if (this->bufsize.has_value())
        throw std::invalid_argument("Duplicated argument detected: bufsize");
    this->bufsize = std::move(bufsize);
}
void PopenConfig::set_value(const types::std_in_t& std_in) {
    if (this->std_in.has_value())
        throw std::invalid_argument("Duplicated argument detected: std_in");
    this->std_in = std_in;
}
void PopenConfig::set_value(types::std_in_t&& std_in) {
    if (this->std_in.has_value())
        throw std::invalid_argument("Duplicated argument detected: std_in");
    this->std_in = std::move(std_in);
}
void PopenConfig::set_value(const types::std_out_t& std_out) {
    if (this->std_out.has_value())
        throw std::invalid_argument("Duplicated argument detected: std_out");
    this->std_out = std_out;
}
void PopenConfig::set_value(types::std_out_t&& std_out) {
    if (this->std_out.has_value())
        throw std::invalid_argument("Duplicated argument detected: std_out");
    this->std_out = std::move(std_out);
}
void PopenConfig::set_value(const types::std_err_t& std_err) {
    if (this->std_err.has_value())
        throw std::invalid_argument("Duplicated argument detected: std_err");
    this->std_err = std_err;
}
void PopenConfig::set_value(types::std_err_t&& std_err) {
    if (this->std_err.has_value())
        throw std::invalid_argument("Duplicated argument detected: std_err");
    this->std_err = std::move(std_err);
}
void PopenConfig::set_value(const types::preexec_fn_t& preexec_fn) {
    if (this->preexec_fn.has_value())
        throw std::invalid_argument("Duplicated argument detected: preexec_fn");
    this->preexec_fn = preexec_fn;
}
void PopenConfig::set_value(types::preexec_fn_t&& preexec_fn) {
    if (this->preexec_fn.has_value())
        throw std::invalid_argument("Duplicated argument detected: preexec_fn");
    this->preexec_fn = std::move(preexec_fn);
}

} // namespace subprocess