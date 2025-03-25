# subprocess

This project is a C++ implementation of Python's `subprocess` module. It provides functionality to create and manage child processes in C++, similar to how `subprocess` works in Python.

🚧 **Work in Progress** 🚧 

Please note that this project is still in development, so some parts may change or may not be functional.

## Requirements

- C++17 or later  
- A POSIX-compliant operating system (e.g., Linux, WSL2 on windows)  
- CMake for building the project


## Installation

### Using CMake

The recommended way to use the `subprocess` library in your own CMake project is by adding it as a subdirectory or an external project. Here's how you can do it:

1. **Clone repository:**
    ```bash
    git clone https://github.com/geb0598/subprocess.git
    cd subprocess
    ```

2. **Add library as a subdirectory in your project’s `CMakeLists.txt`:**

    ```cmake
    add_subdirectory(subprocess)
    ```

3. **Link the subprocess library to your target:**

    In your project's `CMakeLists.txt`, link your target with the `subprocess` library:

    ```cmake
    target_link_libraries(your_target_name subprocess)
    ```

4. **Include the header files in your source files:**

    In your C++ source files, include the necessary header files from `subprocess`:

    ```c++
    #include <subprocess.h>
    ```

---

### Manual Installation (Less Recommended)

If you don’t want to use CMake directly, you can manually copy the required header files and the built static library into your project:

1. **Clone repository:**
    ```bash
    git clone https://github.com/geb0598/subprocess.git
    cd subprocess
    ```

2. **Build library**
    ```bash
    mkdir build
    cd build
    cmake --build .
    ```

3. **Copy the `include` directory** to your project's include path.

4. **Copy the built `subprocess` static library** (e.g., `libsubprocess.a`) to your project and link it manually.

However, using CMake is highly recommended for ease of integration.

## Usage

To use the `subprocess` library, you typically create temporary objects of different types to manage process execution. These types are declared under the namespace `subprocess::types`.

Here are the key types used for process management:

<details>
<summary>types</summary>

### `args_t`

This class represents the command-line arguments for process execution. It allows initialization with a variable number of parameters, which are automatically stored as a `std::vector<std::string>`.

```cpp
args_t("program_name", "args");
```

### `bufsize_t`

This class defines the buffer size for process pipe communication. You can specify:
- `size == 0`: No buffering.
- `size == 1`: Line buffering.
- `size >  1`: Full buffering with the specified size.
- `size <  0`: Full buffering with the default size.

```cpp
bufsize_t(1024);
```

### `std_in_t`, `std_out_t`, `std_err_t`

These classes define how standard input, output, and error are handled for a process. You can redirect the input/output to files, streams, or pipes.

- **`std_in_t`**: Represents the standard input source for a process.
- **`std_out_t`**: Represents the standard output destination for a process.
- **`std_err_t`**: Represents the standard error output destination for a process.

**IOOption**
- **`NONE`**: Default, no redirection.
- **`PIPE`**: Open a pipe for input/output redirection.
- **`STDOUT`**: Used only by std_err_t, redirects standard error to where standard output is directed.
- **`DEVNULL`**: Discards output by redirecting to /dev/null. Only valid for std_out_t and std_err_t.

Example usage:

```cpp
std_in_t("input.txt");  // Redirects input from a file.
std_out_t("output.txt");  // Redirects output to a file.
std_err_t(IOOption::DEVNULL);  // Discards error output.
```

### `preexec_fn_t`

This class allows you to specify a function to be executed after the fork but before executing a new process. It is useful for setting up the environment or modifying process attributes before the new process starts.

Example usage:

```cpp
preexec_fn_t([]() {
    // Modify environment or attributes before forking.
});
```

</details>

### Creating a Process

The `subprocess::PopenConfig` allows you to configure the process creation with flexibility in the order of arguments. You can pass the arguments in any order, and they will be correctly processed. This approach mimics the flexibility of Python's `kwargs`.

```cpp
#include <fstream>

#include <subprocess.h>  

using namespace subprocess;

int main (
    // Create a Popen object using PopenConfig, with flexible argument order
    Popen p(PopenConfig(
        types::args_t("program_name", "args1", "args2"), // Command/Program
        types::std_in_t(input_path),                     // Standard input from file stream
        types::std_out_t(output_path)                    // Standard output to file stream
    ));
)
```
```cpp

    // You can change the order of arguments without issues:
    Popen p(PopenConfig(
        types::std_out_t(input_path),                     // Standard output to file stream
        types::args_t("program_name", "args1", "args2"),  // Command/Program
        types::std_in_t(output_path)                      // Standard input from file stream
    ));
```

## References

- [subprocess](https://github.com/benman64/subprocess)
- [cpp-subprocess](https://github.com/arun11299/cpp-subprocess)
- [Python docs: subprocess](https://docs.python.org/3/library/subprocess.html)