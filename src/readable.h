#ifndef READABLE_H
#define READABLE_H

#include <iostream>

#include <unistd.h>
#include <stdio.h>

namespace subprocess {

class IReadable {
public:
    virtual operator bool() = 0;
    // TODO: Implement Read function
    virtual ssize_t read(void* buf, size_t count) = 0;
    // TODO: Implement Connect function to connect source with pipe
    //       File Reader would be connected through dup2
    //       Otherwise, they would connect standard I/O to pipe, then read from it to simulate the behavior
    // TODO: Implement other functions
    virtual int     seek(int offset, int origin) = 0;
    virtual int     tell() = 0;
};

class FileReader : public IReadable {
public:
    ~FileReader();

    FileReader(int fd, bool is_owner = false);
    FileReader(FILE* fp, bool is_owner = false);

    FileReader            (const FileReader& other) = delete;
    FileReader& operator= (const FileReader& other) = delete;
    FileReader            (FileReader&& other) noexcept;
    FileReader& operator= (FileReader&& other) noexcept;

    virtual operator bool();

    ssize_t read(void* buf, size_t count);
    int     seek(int offset, int origin);
    int     tell();

private:
    void open();
    void close();

    FILE* fp_;
    bool  is_owner_;
};

class PipeReader : public FileReader {
public:

private:

};

class StreamReader : public IReadable {
public:
    ~StreamReader();

    StreamReader(std::istream& stream);

    StreamReader            (const StreamReader& other) = delete;
    StreamReader& operator= (const StreamReader& other) = delete;
    StreamReader            (StreamReader&& other) noexcept;
    StreamReader& operator= (StreamReader&& other) noexcept;

    virtual operator bool();

    ssize_t read(void* buf, size_t count);
    int     seek(int offset, int origin);
    int     tell();

private:
    std::istream& stream_;
};

class StringReader : public IReadable {
public:
    ~StringReader();

    StringReader(std::string& s);

    StringReader            (const StringReader& other) = delete;
    StringReader& operator= (const StringReader& other) = delete;
    StringReader            (StringReader&& other) noexcept;
    StringReader& operator= (StringReader&& other) noexcept;

    virtual operator bool();

    ssize_t read(void* buf, size_t count);
    int     seek(int offset, int origin);
    int     tell();

private:
    std::string& s_;
    size_t       cursor_;
};

}

#endif