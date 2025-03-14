#ifndef BYTES_H
#define BYTES_H

#include <vector>
#include <string>

namespace subprocess {

class Bytes {
public:
    using value_type = char;
    using size_type  = std::vector<value_type>::size_type;

    ~Bytes()                      = default;
    Bytes()                       = default;
    Bytes(const Bytes& other)     = default;
    Bytes(Bytes&& other) noexcept = default;
    Bytes(size_type n, value_type val = value_type());
    template <
        typename InputIterator,
        typename = std::enable_if_t <
            std::is_convertible_v <
                typename std::iterator_traits<InputIterator>::iterator_category, 
                std::input_iterator_tag
            >
        >
    > Bytes(InputIterator first, InputIterator last) : bytes_(first, last) {}

    Bytes&            operator=(const Bytes& other)     = default;
    Bytes&            operator=(Bytes&& other) noexcept = default;

    value_type&       operator[](size_type n);
    const value_type& operator[](size_type n) const;

    size_type         size() const;
    bool              empty() const;
    void              resize(size_type n, value_type val = value_type());

    void              clear();
    void              push_back(const value_type& value);
    void              push_back(value_type&& value);

    value_type*       data();
    const value_type* data() const;

private:
    std::vector<value_type> bytes_;
};

}

#endif