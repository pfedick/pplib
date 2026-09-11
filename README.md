# PPLib - Patrick's Programming Library

## INDEX

- [What is PPLib](##What-is-PPLib)
- [Installation](##Installation)
- [Documentation](##Documentation)
- [License](##License)


## What is PPLib

PPLib is an acronym for "Patrick's Programming Library". It is a C++ library that provides a wide range of utilities and abstractions to simplify common programming tasks. It covers file and directory functions, strings and arrays, time, math, threads, graphics, sound, database access, and internet communication.

You can find the newest version and documentation of this library in GitHub:

- https://github.com/patrickf/pplib


## Installation

PPLib uses CMake as its build system.

To install PPLib using CMake, follow these steps:

1. Clone the repository:

    ```bash
    git clone https://github.com/patrickf/pplib.git
    cd pplib
    ```

2. Run CMake to configure the build:

    ```bash
    cmake -B build/release -DCMAKE_BUILD_TYPE=Release
    ```

4. Build and install the library:

   ```bash
   cmake --build build/release -j
   cmake --install build/release
   ```

### Compile and Install specific components

To compile and install specific components of PPLib, you can use the `-D` option with CMake to enable or disable certain features. For example:

```bash
# TODO: Describe how this works
cmake -B build/release -DCMAKE_BUILD_TYPE=Release -DENABLE_COMPONENT_X=ON
cmake --build build/release -j 
cmake --install build/release
```

Available Componets are:

### Compile on Raspberry Pico
TODO

### Use as a Submodule in another project

TODO!

To use PPLib as a submodule in another project, follow these steps:

1. Add PPLib as a submodule:

    ```bash
    git submodule add https://github.com/patrickf/pplib.git external/pplib
    git submodule update --init --recursive
    ```

2. Include PPLib in your CMake project:

    ```cmake
    add_subdirectory(external/pplib)
    target_link_libraries(your_project PRIVATE pplib)
    ```

   

## Documentation
All classes and functions are documented in the header files. Debending on your code editor, you should be able to access this documentation directly while coding.

You can also find a complete HTML documentation of PPLib on the PPLib webpage:

- https://www.pplib.de/docs

You can also generate it yourself in many formats by running the Doxygen tool on the source code:

```bash
mkdir -p documentation/html
doxygen Doxyfile
cp docs/header-bg.png documentation/html
```
   



## License
```
Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer. 
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```


