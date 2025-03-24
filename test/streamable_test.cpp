#include <filesystem>
#include <fstream>
#include <random>

#include <gtest/gtest.h>

#include "subprocess/streamable.h"

/* ===================================== File Test ===================================== */
class StreamableFileTest : public ::testing::Test {
protected:
    virtual void SetUp() override {
        std::ofstream src_file(src, std::ios::out | std::ios::trunc);
        if (!src_file)
            std::cerr << "Failed to open file: " << src << std::endl;
        src_file.write(input.c_str(), input.size());
        src_file.close();

        std::ofstream dest_file(dest);
        dest_file.close();

        in.open(std::fopen(src.c_str(), "r"));
        out.open(std::fopen(dest.c_str(), "w")); 
    }

    virtual void TearDown() override {
        in.close();
        out.close();
    }

    size_t read_all() {
        std::ifstream dest_file(dest, std::ios::in);
        if (!dest_file) 
            std::cerr << "Failed to open file: " << dest << std::endl;
        dest_file.seekg(0, std::ios::end);
        size_t size = dest_file.tellg();
        dest_file.seekg(0, std::ios::beg);

        output.resize(size);
        dest_file.read(output.data(), size);
        dest_file.close();
        return size;
    }

    std::filesystem::path src    = "./test/in_test.txt";
    std::filesystem::path dest   = "./test/out_test.txt";
    std::string           input  = "Hello World!";
    std::string           output;
    subprocess::File      in;
    subprocess::File      out;
};

TEST_F(StreamableFileTest, ReadTest) {
    EXPECT_TRUE(in.is_opened());
    EXPECT_TRUE(in.is_readable());
    EXPECT_FALSE(in.is_writable());

    size_t size_to_read = 5;
    subprocess::Bytes output_bytes = in.read(size_to_read);
    EXPECT_GE(size_to_read, output_bytes.size());
    for (int i = 0; i < output_bytes.size(); ++i) {
        EXPECT_EQ(input[i], output_bytes[i]) << i << "th element";
    }
}

TEST_F(StreamableFileTest, ReadAllTest) {
    EXPECT_TRUE(in.is_opened());
    EXPECT_TRUE(in.is_readable());
    EXPECT_FALSE(in.is_writable());

    subprocess::Bytes output_bytes = in.read_all();
    ASSERT_EQ(input.size(), output_bytes.size());
    for (int i = 0; i < input.size(); ++i) {
        EXPECT_EQ(input[i], output_bytes[i]) << i << "th element";
    }
}

TEST_F(StreamableFileTest, WriteTest) {
    EXPECT_TRUE(out.is_opened());
    EXPECT_FALSE(out.is_readable());
    EXPECT_TRUE(out.is_writable());

    subprocess::Bytes input_bytes(input.begin(), input.end());
    ASSERT_EQ(input.size(), input_bytes.size());

    out.write(input_bytes, input_bytes.size());

    size_t size_read = read_all();
    EXPECT_EQ(size_read, input_bytes.size());
    for (int i = 0; i < size_read; ++i) {
        EXPECT_EQ(input_bytes[i], output[i]) << i << "th element";
    }
}

/* ===================================== IOStream Test ===================================== */
class StreamableIOStreamTest : public ::testing::Test {
protected:
    virtual void SetUp() override {
        input_stream.str(input);
        in.open(&input_stream);
        out.open(&output_stream);
    }

    virtual void TearDown() override {
        in.close();
        out.close();
    }

    size_t read_all() {
        output = output_stream.str();
        return output.size();
    }

    std::string          input  = "Hello World!";
    std::string          output;
    std::stringstream    input_stream;
    std::stringstream    output_stream;
    subprocess::IOStream in;
    subprocess::IOStream out;
};

TEST_F(StreamableIOStreamTest, ReadTest) {
    EXPECT_TRUE(in.is_opened());
    EXPECT_TRUE(in.is_readable());

    size_t size_to_read = 5;
    subprocess::Bytes output_bytes = in.read(size_to_read);
    EXPECT_GE(size_to_read, output_bytes.size());
    for (int i = 0; i < output_bytes.size(); ++i) {
        EXPECT_EQ(input[i], output_bytes[i]) << i << "th element";
    }
}

TEST_F(StreamableIOStreamTest, ReadAllTest) {
    EXPECT_TRUE(in.is_opened());
    EXPECT_TRUE(in.is_readable());

    subprocess::Bytes output_bytes = in.read_all();
    EXPECT_GE(input.size(), output_bytes.size());
    for (int i = 0; i < output_bytes.size(); ++i) {
        EXPECT_EQ(input[i], output_bytes[i]) << i << "th element";
    }
}

TEST_F(StreamableIOStreamTest, WriteTest) {
    EXPECT_TRUE(out.is_opened());
    EXPECT_TRUE(out.is_writable());

    subprocess::Bytes input_bytes(input.begin(), input.end());
    ASSERT_EQ(input.size(), input_bytes.size());

    out.write(input_bytes, input_bytes.size());

    size_t size_read = read_all();

    EXPECT_EQ(size_read, output.size());
    for (int i = 0; i < size_read; ++i) {
        EXPECT_EQ(input_bytes[i], output[i]) << i << "th element";
    }
}

/* ===================================== Communicate Test ===================================== */

class StreamableCommunicateTest : public ::testing::Test {
protected:
    virtual void SetUp() override {
        input_stream.str(input);
        in.open(&input_stream);
        out.open(&output_stream);
    }

    virtual void TearDown() override {
        in.close();
        out.close();
    }

    size_t read_all() {
        output = output_stream.str();
        return output.size();
    }

    void generate_input(size_t size) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 25);

        input.clear();
        input.resize(size);
        for (int i = 0; i < size; ++i) {
            input[i] = dis(gen) + 'a';
        }
        input_stream.str(input);
    }

    std::string          input  = "Hello World!";
    std::string          output;
    std::stringstream    input_stream;
    std::stringstream    output_stream;
    subprocess::IOStream in;
    subprocess::IOStream out;
};

TEST_F(StreamableCommunicateTest, CommunicateTest) {
    generate_input(10000); 
    size_t size_written = subprocess::communicate(in, out);
    EXPECT_EQ(input.size(), size_written);
    size_t size_read = read_all();
    ASSERT_EQ(size_written, size_read);
    for (int i = 0; i < size_written; ++i) {
        EXPECT_EQ(input[i], output[i]) << i << "th element";
    }
}

TEST_F(StreamableCommunicateTest, CommunicateAsyncTest) {
    generate_input(100'000'000);
    auto future_size = subprocess::communicate_async(in, out);
    size_t size_written;
    try {
        size_written = future_size.get();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        FAIL();
    }
    EXPECT_EQ(input.size(), size_written);
    size_t size_read = read_all();
    ASSERT_EQ(size_written, size_read);
    for (int i = 0; i < size_written; ++i) {
        EXPECT_EQ(input[i], output[i]) << i << "th element";
    }
}