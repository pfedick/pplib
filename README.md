# PPLib - Patrick's Programming Library

## INDEX

- [What is PPLib](##What-is-PPLib)
- [Installation](##Installation)
- [Usage](##Usage)
- [Documentation](##Documentation)
- [License](##License)


## What is PPLib

PPLib is an acronym for "Patrick's Programming Library". It is a C++ library that provides a wide range of utilities and abstractions to simplify common programming tasks. It covers file and directory functions, strings and arrays, time, math, threads, graphics, sound, database access, and internet communication.

You can find the newest version and documentation of this library in GitHub:

- https://github.com/pfedick/pplib


## Installation

PPLib uses CMake as its build system.

To install PPLib using CMake, follow these steps:

1. Clone the repository:

    ```bash
    git clone https://github.com/pfedick/pplib.git
    cd pplib
    ```

2. Run CMake to configure the build:

    ```bash
    cmake -B build/release -DCMAKE_BUILD_TYPE=Release
    ```

3. Build and install the library:

   ```bash
   cmake --build build/release -j
   cmake --install build/release
   ```

### Compile and install specific components

All modules are built by default. Enable or disable modules with these CMake options:

- `PPLIB_ENABLE_CORE`, `PPLIB_ENABLE_CRYPTO`, `PPLIB_ENABLE_GRAFIX`,
  `PPLIB_ENABLE_AUDIO`, `PPLIB_ENABLE_DATABASE`, `PPLIB_ENABLE_INTERNET` (all default `ON`)
- `PPLIB_ENABLE_PICO` (default `OFF`) — Raspberry Pico module

Optional external libraries (all default `ON`; if the library is not installed, the
related features are compiled out automatically): `PPLIB_WITH_ASM`,
`PPLIB_WITH_OPENSSL`, `PPLIB_WITH_CURL`, `PPLIB_WITH_JPEG`, `PPLIB_WITH_PNG`,
`PPLIB_WITH_TIFF`, `PPLIB_WITH_FREETYPE`, `PPLIB_WITH_SQLITE3`, `PPLIB_WITH_MYSQL`,
`PPLIB_WITH_POSTGRES`, `PPLIB_WITH_MPG123`, `PPLIB_WITH_LAME`, `PPLIB_WITH_VORBIS`,
`PPLIB_WITH_MICROHTTPD`.

Example — build without the audio module and without TIFF support:

```bash
cmake -B build/release -DCMAKE_BUILD_TYPE=Release -DPPLIB_ENABLE_AUDIO=OFF -DPPLIB_WITH_TIFF=OFF
cmake --build build/release -j
cmake --install build/release    # installs into /usr/local by default
```

### Installation prefix

The default install prefix is `/usr/local` and requires root privileges for
`cmake --install`. To install as an unprivileged user, pick a prefix you own:

```bash
cmake -B build/home -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build/home -j
cmake --install build/home
```

The provided `Makefile` covers both cases as well:

```bash
make release                        # build release binaries
make install PREFIX=/usr/local      # system wide install
make install_home                   # install into $HOME
```

### Compile on Raspberry Pico
TODO

## Usage

PPLib consists of independent modules — link only the ones you actually need.
Inter-module dependencies are resolved automatically: `pplib::crypto` pulls in
`pplib::core`, and `pplib8-crypto` lists `pplib8-core` in its pkg-config `Requires:`.

| Module   | Description                           | CMake target      | pkg-config module  |
|----------|---------------------------------------|-------------------|--------------------|
| Core     | Core utilities, types and strings     | `pplib::core`     | `pplib8-core`      |
| Crypto   | Cryptographic functions, CSPRNG       | `pplib::crypto`   | `pplib8-crypto`    |
| Grafix   | Graphics rendering and manipulation   | `pplib::grafix`   | `pplib8-grafix`    |
| Audio    | Audio processing and playback         | `pplib::audio`    | `pplib8-audio`     |
| DB       | Database access                       | `pplib::db`       | `pplib8-database`  |
| Internet | Internet communication                | `pplib::internet` | `pplib8-internet`  |

The meta module `pplib8` (pkg-config) covers all installed modules at once.

If PPLib is installed outside the default search paths of your toolchain:

- pkg-config: `export PKG_CONFIG_PATH=<prefix>/lib/pkgconfig`
- CMake: `-DCMAKE_PREFIX_PATH=<prefix>`
- `pplib8-config` (installed into `<prefix>/bin`) sets `PKG_CONFIG_PATH` for its
  own prefix automatically, so it works even without any environment setup



### CMake project with pplib as git submodule

To use PPLib as a submodule in another project, follow these steps:

1. Add PPLib as a submodule:

    ```bash
    git submodule add https://github.com/pfedick/pplib.git external/pplib
    git submodule update --init --recursive
    ```

2. Include PPLib in your CMake project:

    ```cmake
    add_subdirectory(external/pplib)
    # Link only the modules you actually use; pplib::core is pulled in automatically:
    target_link_libraries(your_project PRIVATE pplib::crypto)
    ```

No install step is needed, the library is built as part of your project.


### CMake project against an installed PPLib

```bash
cmake -B build -DCMAKE_PREFIX_PATH=/usr/local ..
```

```cmake
find_package(pplib8 REQUIRED)
target_link_libraries(your_project PRIVATE pplib::crypto)
```

`find_package(pplib8)` provides a target for every installed module
(`pplib::core`, `pplib::crypto`, `pplib::grafix`, `pplib::internet`, `pplib::db`,
`pplib::audio`) — just link the ones you need. All are static libraries; their
transitive link dependencies (`pplib::core`, OpenSSL, zlib, …) are attached
automatically. For a non-standard install prefix, point `CMAKE_PREFIX_PATH` to it.

If you prefer pkg-config over the CMake package files:

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(PPLIB REQUIRED pplib8-crypto)      # or pplib8 for all modules
target_include_directories(your_project PRIVATE ${PPLIB_INCLUDE_DIRS})
target_link_directories(your_project PRIVATE ${PPLIB_LIBRARY_DIRS})
target_link_libraries(your_project ${PPLIB_LIBRARIES})
```

### Autoconf / Automake project

In `configure.ac`, require only the modules you use (version constraint optional):

```m4
PKG_CHECK_MODULES([PPLIB_CRYPTO], [pplib8-crypto >= 8.0])
```

In `Makefile.am`:

```make
myapp_CPPFLAGS = $(PPLIB_CRYPTO_CFLAGS)
myapp_LDADD    = $(PPLIB_CRYPTO_LIBS)
```

`PKG_CHECK_MODULES` resolves inter-module dependencies (e.g. `pplib8-crypto` →
`pplib8-core`) and produces the correct static link order. PPLib is built as
static libraries only, so `pkg-config --static` is never required — every
link-time dependency is already listed in the module's `Requires:`.

### Plain Makefile project

Ask pkg-config for the flags of the module you use:

```make
CXXFLAGS += $(shell pkg-config --cflags pplib8-crypto)
LDLIBS   += $(shell pkg-config --libs pplib8-crypto)
```

Use `pplib8` instead of `pplib8-crypto` to link all installed modules. Note that
`$(LDLIBS)` must appear *after* the object files on the link command line —
static libraries are order-sensitive, and `pkg-config` already outputs the
correct order.

If pkg-config is not available on your system, the installed `pplib8-config`
wrapper works without it (it always covers all installed modules):

```make
CXXFLAGS += $(shell pplib8-config --cflags)
LDLIBS   += $(shell pplib8-config --libs)
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


