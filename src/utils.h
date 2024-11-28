#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace subprocess {

namespace utils {

struct Concatenator {
    Concatenator(std::string delimiter = " ") : delimiter_(delimiter) {} 
    template <typename... Args>
    std::string concat(std::string s, Args... args) { s + delimiter + concat(args...); }
    std::string concat(std::string s) { return s; };
    std::string delimiter_;
};

}

}

#endif