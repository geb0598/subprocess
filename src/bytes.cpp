#include "bytes.h"

namespace subprocess {

Bytes::Bytes(size_type n, value_type val) : bytes_(n, val) {}

Bytes::value_type& Bytes::operator[](size_type n) {
    return bytes_[n];
}

const Bytes::value_type& Bytes::operator[](size_type n) const {
    return bytes_[n];
}

Bytes::size_type Bytes::size() const {
    return bytes_.size();
}

bool Bytes::empty() const {
    return bytes_.empty();
}

void Bytes::resize(size_type n, value_type val) {
    bytes_.resize(n, val);
}

void Bytes::clear() {
    bytes_.clear();
}

void Bytes::push_back(const value_type& value) {
    bytes_.push_back(value);
}

void Bytes::push_back(value_type&& value) {
    bytes_.push_back(std::move(value));
}

Bytes::value_type* Bytes::data() {
    return bytes_.data();
}

const Bytes::value_type* Bytes::data() const {
    return bytes_.data();
}

char* Bytes::c_str() {
    return static_cast<char*>(data());
}

const char* Bytes::c_str() const {
    return static_cast<const char*>(data());
}

}