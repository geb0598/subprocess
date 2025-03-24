#include <fstream>

#include <gtest/gtest.h>

#include "bytes.h"

TEST(ByteTest, ConstructWithIterator) {
    std::string s = "Hello World!";
    subprocess::Bytes bytes(s.begin(), s.end());
    for (int i = 0; i < s.size(); ++i) {
        EXPECT_EQ(s[i], bytes[i]) << i << "th element";
    }
}