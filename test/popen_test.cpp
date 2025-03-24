#include <iostream>
#include <fstream>
#include <random>

#include <gtest/gtest.h>

#include "popen.h"

TEST(PopenConfigTest, ConstructorTest) {
    subprocess::PopenConfig config (
        subprocess::types::args_t("1", "2", "3")
    );

    std::vector<std::string> v = { "1", "2", "3" };
    for (int i = 0; i < v.size(); ++i) {
        EXPECT_EQ(config.args.value().args[i], v[i]);
    }
}

TEST(PopenTest, ExecTest) {
    subprocess::Popen p(subprocess::PopenConfig(
        subprocess::types::args_t("test/helpers/hello_world")
    ));
    
    p.wait();

    ASSERT_EQ(p.returncode().value(), EXIT_SUCCESS);
}

TEST(PopenTest, ReturncodeTest) {
    int returncode = 100;
    subprocess::Popen p(subprocess::PopenConfig(
        subprocess::types::args_t("test/helpers/return", std::to_string(returncode))
    ));
    
    p.wait();

    ASSERT_EQ(p.returncode().value(), returncode);
}

TEST(PopenTest, SignalTest) {
    subprocess::Popen p(subprocess::PopenConfig(
        subprocess::types::args_t("test/helpers/sleep")
    ));

    p.send_signal(SIGTERM);
    p.wait();
 
    ASSERT_EQ(p.returncode().value(), -SIGTERM);
}

TEST(PopenTest, PipeTest) {
    subprocess::Popen p(subprocess::PopenConfig(
        subprocess::types::args_t("test/helpers/echo"),
        subprocess::types::std_in_t(subprocess::types::IOOption::PIPE),
        subprocess::types::std_out_t(subprocess::types::IOOption::PIPE)
    ));

    std::string s = "Hello World!";
    subprocess::Bytes input(s.begin(), s.end());
    auto [std_out_data, std_err_data] = p.communicate(input, 3);

    ASSERT_EQ(p.returncode().value(), EXIT_SUCCESS);
    ASSERT_TRUE(std_out_data.has_value());
    EXPECT_EQ(input.size(), std_out_data.value().size());
    for (int i = 0; i < std::min(input.size(), std_out_data.value().size()); ++i) {
        EXPECT_EQ(input[i], std_out_data.value()[i]) << i << "th element";
    }
}

class PopenFileTest : public ::testing::Test {
protected:
    virtual void SetUp() override {
        std::ofstream src_file(src, std::ios::out | std::ios::trunc);
        if (!src_file)
            std::cerr << "Failed to open file: " << src << std::endl;
        src_file.write(input.c_str(), input.size());
        src_file.close();

        std::ofstream dest_file(dest);
        dest_file.close();
    }

    virtual void TearDown() override {}

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

    void generate_input(size_t size) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 25);

        input.clear();
        input.resize(size);
        for (int i = 0; i < size; ++i) {
            input[i] = dis(gen) + 'a';
        }
        std::ofstream src_file(src, std::ios::out | std::ios::trunc);
        if (!src_file)
            std::cerr << "Failed to open file: " << src << std::endl;
        src_file.write(input.c_str(), input.size());
        src_file.close();
    }

    std::filesystem::path src    = "test/in.txt";
    std::filesystem::path dest   = "test/out.txt";
    std::string           input  = "Hello World!";
    std::string           output;
};

TEST_F(PopenFileTest, FILETest) {
    generate_input(10);

    FILE* src_fp  = ::fopen(src.c_str(), "r");
    FILE* dest_fp = ::fopen(dest.c_str(), "w");
    subprocess::Popen p(subprocess::PopenConfig(
        subprocess::types::args_t("test/helpers/echo"),
        subprocess::types::std_in_t(src_fp),
        subprocess::types::std_out_t(dest_fp)
    ));

    ASSERT_TRUE(p.wait());

    ASSERT_EQ(p.returncode().value(), EXIT_SUCCESS);

    read_all();

    EXPECT_EQ(input.size(), output.size());
    for (int i = 0; i < std::min(input.size(), output.size()); ++i) {
        EXPECT_EQ(input[i], output[i]) << i << "th element";
    }
}

TEST_F(PopenFileTest, fstreamTest) {
    generate_input(1000000);

    std::ifstream src_stream(src);
    std::ofstream dest_stream(dest);
    subprocess::Popen p(subprocess::PopenConfig(
        subprocess::types::args_t("test/helpers/echo"),
        subprocess::types::std_in_t(&src_stream),
        subprocess::types::std_out_t(&dest_stream)
    ));

    ASSERT_TRUE(p.wait()); 

    ASSERT_EQ(p.returncode().value(), EXIT_SUCCESS);

    read_all();

    EXPECT_EQ(input.size(), output.size());
    for (int i = 0; i < std::min(input.size(), output.size()); ++i) {
        EXPECT_EQ(input[i], output[i]) << i << "th element";
    }
}

// TODO: fstream use async communication