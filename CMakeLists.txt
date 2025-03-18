cmake_minimum_required(VERSION 3.10)
project(bin2hex C CXX)

# Enable Ccache support
find_program(CCACHE_PROGRAM ccache REQUIRED)
if(NOT CCACHE_PROGRAM)
    message(FATAL_ERROR "ccache not found! Please install ccache or adjust the configuration.")
endif()

# On Linux, using GNU toolchain, force static linking for the executable.
if(NOT (UNIX AND NOT APPLE))
    message(FATAL_ERROR "Unknown Operating System")
endif()

# Set ccache as the compiler launcher before setting the compiler
set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")

# Optionally, set the C and C++ compilers if you need specific versions
# set(CMAKE_C_COMPILER "/usr/bin/gcc" CACHE STRING "C Compiler" FORCE)
# set(CMAKE_CXX_COMPILER "/usr/bin/g++" CACHE STRING "C++ Compiler" FORCE)

# Set C and C++ standards
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable position-independent code if needed
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set visibility properties
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Universal compiler and linker flags
set(compiler_options
        # Warnings and diagnostics
        -Wall                                   # Enable common warnings
        -Wextra                                 # Enable extra warnings
        -Wpedantic                              # Strict compliance with the standard
        -Wunused                                # Warn about unused variables, functions, etc.
        -Wuninitialized                         # Warn if variables are used uninitialized
        -fdiagnostics-show-option               # Show which option triggered the warning
        -fdiagnostics-color=always              # Enable colored diagnostics for better readability

        # Debugging and stack protection
        -g3                                     # Maximum debug information, including macro expansions
        -O0
        -fstack-usage                           # Generate stack usage info for each function
        -fstack-protector-all                   # Protect all functions with a stack canary to prevent stack overflow attacks
        # -D_FORTIFY_SOURCE=2                   # Buffer overflow detection on safer libc functions (e.g., memcpy).
        # You need to enable optimization for _FORTIFY_SOURCE to work!
        -gdwarf-4                               # Generate DWARF version 4 debug information

        -fno-eliminate-unused-debug-types
        -fno-omit-frame-pointer
        -lasan -lubsan -fPIC --pie
        -Wl,-z,relro -Wl,-z,now
        -fstack-protector-all
)

set(linker_options
        # Linker options for memory safety, thread safety, and verbose debugging
        -Wl,--no-omagic                         # Prevent the generation of object files in memory; useful for debugging
        -Wl,--as-needed                         # Only link libraries that are actually needed to reduce binary size
        -Wl,--fatal-warnings                    # Treat all linker warnings as errors to catch issues early
        -Wl,-z,relro                            # Read-only relocations to prevent certain memory exploits (optional)
        -Wl,-z,now                              # Fully resolve all symbols during the link time for extra safety
        -Wl,-z,noexecstack                      # Prevent execution of code on the stack (security hardening)
        -Wl,-z,defs                             # Ensure all symbols are defined, and prevent undefined symbols
        -Wl,-O0

        -gdwarf-4                               # Generate detailed debug information for the linker
        -fno-eliminate-unused-debug-types
        -fno-omit-frame-pointer

        # Stack protection
        -fstack-protector-all                   # Link with stack protection for all functions
        -lasan -lubsan -fPIC --pie --whole-file
        -Wl,-z,relro -Wl,-z,now
)

if ("${COMPILE_WITH_MEMORY_SANITIZERS}" STREQUAL "True")
    message(STATUS "Sanitizers for memory enabled")
    list(APPEND compiler_options
            -fsanitize=address                  # Detect illegal memory access such as buffer overflows and use-after-free
            -fsanitize=undefined                # Detect undefined behavior like integer overflows and null dereferencing
            # Code coverage options
            -fprofile-arcs                      # Enable code coverage instrumentation
            -ftest-coverage                     # Generate coverage test data
    )

    list(APPEND linker_options
            -fsanitize=address                  # Link the AddressSanitizer runtime for memory integrity
            -fsanitize=undefined                # Link the UndefinedBehaviorSanitizer for detecting undefined behavior
            # Code coverage options
            -fprofile-arcs                      # Enable code coverage instrumentation
            -ftest-coverage                     # Generate coverage test data
    )
endif ()

set(optimization_flags
        -O3
        -march=native
        -mtune=native
        -flto
        -fomit-frame-pointer
        -ffast-math
        -fstrict-aliasing
        -fdata-sections
        -ffunction-sections
        -D_FORTIFY_SOURCE=2
        -fstack-protector-strong
        -Wl,-z,relro -Wl,-z,now
        -s
)

set(optimization_link_flags
        -O3
        -march=native
        -mtune=native
        -flto
        -fomit-frame-pointer
        -ffast-math
        -fstrict-aliasing
        -fdata-sections
        -ffunction-sections
        -Wl,--gc-sections
        -D_FORTIFY_SOURCE=2
        -fstack-protector-strong
        -Wl,-z,relro -Wl,-z,now
        -s
)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_options(${compiler_options})
    add_link_options(${linker_options})
    add_compile_definitions(__DEBUG__)
else ()
    add_compile_options(${optimization_flags})
    add_compile_options(${optimization_link_flags})
    if ("${OVERRIDE_DEFAULT_SIGSEGV}" STREQUAL "True")
        add_compile_definitions(OVERRIDE_DEFAULT_SIGSEGV)
    endif()
endif ()

if ("${COMPILE_LOWER_CASE_MAP}" STREQUAL "True")
    message(STATUS "Using lowercase map")
    add_compile_definitions(LOWER_CASE_MAP)
endif()

function(strip)
    foreach (target IN LISTS ARGN)
        add_custom_target(strip_${target} ALL
                COMMAND strip ${target}
                COMMENT "Stripping ${target}..."
        )

        add_dependencies(strip_${target} ${target})
    endforeach ()
endfunction()

add_library(log OBJECT src/log.cpp src/include/log.hpp)
add_library(libbin2hex OBJECT src/pipe.cpp src/include/pipe.hpp)

include_directories(src/include)
add_executable(bin2hex src/bin2hex.cpp)
add_executable(hex2bin src/hex2bin.cpp)

target_link_libraries(bin2hex PRIVATE log libbin2hex)
target_link_libraries(hex2bin PRIVATE log libbin2hex)

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    strip(bin2hex hex2bin)
endif ()
