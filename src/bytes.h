#ifndef BYTES_H
#define BYTES_H

#include <cstdint>
#include <vector>
#include <string>

namespace subprocess {

class Bytes {
public:
    using size_type  = size_t;
    using value_type = uint8_t;

    ~Bytes();

    Bytes();
    Bytes(size_type n, const value_type& val = value_type());
    Bytes(const std::string& s);
    Bytes(std::string&& s);

    Bytes            (const Bytes& other);
    Bytes& operator= (const Bytes& other);
    Bytes            (Bytes&& other) noexcept;
    Bytes& operator= (Bytes&& other) noexcept;

    value_type&       operator[](size_type n);
    const value_type& operator[](size_type n) const;

    std::string       decode() const;
    std::u8string     decode_u8() const;
    std::u16string    decode_u16() const;
    std::u32string    decode_u32() const;

    size_type         size() const;
    size_type         max_size() const;
    size_type         capacity() const;
    void              resize(size_type n, value_type val = value_type());
    void              reserve(size_type n);
    void              shrink_to_fit();
    bool              empty() const;

private:
    std::vector<value_type> buffer_;
};

}

#endif