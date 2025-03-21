#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <system_error>

namespace subprocess {

/** @brief C++ version of Python's OSError, derived from std::system_error.
 * 
 *  Refer to the std::system_error constructor for details on all other parameters.
 * 
 *  @param file The first file name involved in the operation (optional).
 *  @param file2 The second file name involved in the operation (optional).
 * 
 *  For exceptions that involve a file system path (such as open()),`file` is 
 *  the file name passed to the function. For functions that involve two file 
 *  system paths (such as rename()), `file2` corresponds to the second file name 
 *  passed to the function.
 */
class OSError : public std::system_error {
public:
    OSError (
        int                          ev, 
        const std::error_category&   ecat,
        const std::string&           what_arg, 
        const std::filesystem::path& file  = std::filesystem::path(),
        const std::filesystem::path& file2 = std::filesystem::path()
    ) : std::system_error(ev, ecat, what_arg) {
        std::stringstream ss;
        ss << "[" << std::system_error::code() << "] " << std::system_error::what() << "\n";
        if (!file.empty()) {
            ss << "file: " << std::filesystem::absolute(file) << "\n";
        }
        if (!file2.empty()) {
            ss << "file2: " << std::filesystem::absolute(file) << "\n";
        }
        message_ = ss.str();
    }

    const char* what() const noexcept { return message_.c_str(); }
private:
    std::string message_;
};

// TODO: TimeExpried exception class

}

#endif