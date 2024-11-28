#ifndef WRITABLE_H
#define WRITABLE_H

#include <iostream>
#include <filesystem>

#include <unistd.h>
#include <stdio.h>

namespace subprocess {

class IWritable {
public:
    virtual operator bool() = 0;
    // TODO: Implement Writ function
    virtual ssize_t write(void* buf, size_t count) = 0;
    // TODO: Implement Connect function to connect source with pipe
    //       File Writer would be connected through dup2
    //       Otherwise, they would connect standard I/O to pipe, then read from it to simulate the behavior
    // TODO: Implement other functions
    virtual int     seek(int offset, int origin) = 0;
    virtual int     tell() = 0;
};

class FileWriter : public IWritable {
public:
    ~FileWriter();

    FileWriter(const std::string& path);
    FileWriter(const std::filesystem::path& path);
    FileWriter(int fd, bool is_owner = false);
    FileWriter(FILE* fp, bool is_owner = false);

    FileWriter            (const FileWriter& other) = delete;
    FileWriter& operator= (const FileWriter& other) = delete;
    FileWriter            (FileWriter&& other) noexcept;
    FileWriter& operator= (FileWriter&& other) noexcept;

    virtual operator bool();

    ssize_t write(void* buf, size_t count);
    int     seek(int offset, int origin);
    int     tell();

private:
    void open();
    void close();

    FILE* fp_;
    bool  is_owner_;
};

class PipeWriter : public FileWriter {

};

class StreamWriter : public IWritable {
public:
    ~StreamWriter();

    StreamWriter(std::ostream& stream);

    StreamWriter            (const StreamWriter& other) = delete;
    StreamWriter& operator= (const StreamWriter& other) = delete;
    StreamWriter            (StreamWriter&& other) noexcept;
    StreamWriter& operator= (StreamWriter&& other) noexcept;

    virtual operator bool();

    ssize_t write(void* buf, size_t count);
    int     seek(int offset, int origin);
    int     tell();

private:
    std::ostream& stream_;
};

class StringWriter : public IWritable {
public:
    ~StringWriter();

    StringWriter(std::string& s);

    StringWriter            (const StringWriter& other) = delete;
    StringWriter& operator= (const StringWriter& other) = delete;
    StringWriter            (StringWriter&& other) noexcept;
    StringWriter& operator= (StringWriter&& other) noexcept;

    virtual operator bool();

    ssize_t write(void* buf, size_t count);
    int     seek(int offset, int origin);
    int     tell();

private:
    std::string& s_;
    size_t       cursor_;
};

}

#endif