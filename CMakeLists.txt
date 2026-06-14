# axon-lang CMakeLists.txt
# Builds the AXON compiler (axonc) -- C core with optional LLVM backend
# APSE Group -- Engineering Directorate

cmake_minimum_required(VERSION 3.25)
project(axon-compiler
    VERSION 0.1.0
    DESCRIPTION "AXON Language Compiler -- Zero-Copy Cognitive Graph Language"
    LANGUAGES C
)

# -----------------------------------------------------------------------
# Global compiler settings
# -----------------------------------------------------------------------
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Aggressive optimization for release
set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native -flto -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_DEBUG   "-O1 -g3 -fsanitize=address,undefined")

# Common warnings
add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wshadow -Wstrict-prototypes
    -Wno-unused-parameter
)

# -----------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------
set(AXON_COMPILER_SOURCES
    compiler/src/lexer/lexer.c
    compiler/src/parser/parser.c
    compiler/src/typechecker/typechecker.c
    compiler/src/codegen/codegen.c
    compiler/src/optimizer/optimizer.c
    compiler/src/main.c
)

# -----------------------------------------------------------------------
# Include directories
# -----------------------------------------------------------------------
include_directories(
    compiler/src/lexer
    compiler/src/parser
    compiler/src/typechecker
    compiler/src/codegen
    compiler/src/optimizer
    compiler/include
)

# -----------------------------------------------------------------------
# axonc executable
# -----------------------------------------------------------------------
add_executable(axonc ${AXON_COMPILER_SOURCES})

target_compile_definitions(axonc PRIVATE
    AXON_VERSION_MAJOR=0
    AXON_VERSION_MINOR=1
    AXON_VERSION_PATCH=0
)

# -----------------------------------------------------------------------
# Optional: LLVM backend
# -----------------------------------------------------------------------
option(AXON_LLVM_BACKEND "Enable LLVM IR codegen backend" OFF)

if(AXON_LLVM_BACKEND)
    find_package(LLVM 17 REQUIRED CONFIG)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})
    llvm_map_components_to_libnames(LLVM_LIBS
        core support irreader passes
        x86codegen x86asmparser
        aarch64codegen riscvcodegen
    )
    target_link_libraries(axonc PRIVATE ${LLVM_LIBS})
    target_compile_definitions(axonc PRIVATE AXON_LLVM=1)
endif()

# -----------------------------------------------------------------------
# Install targets
# -----------------------------------------------------------------------
install(TARGETS axonc
    RUNTIME DESTINATION bin
)

install(DIRECTORY compiler/include/
    DESTINATION include/axon
    FILES_MATCHING PATTERN "*.h"
)

# -----------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------
enable_testing()

if(EXISTS "${CMAKE_SOURCE_DIR}/compiler/tests")
    add_subdirectory(compiler/tests)
endif()

message(STATUS "axon-lang compiler build configured")
message(STATUS "  Build type:    ${CMAKE_BUILD_TYPE}")
message(STATUS "  LLVM backend:  ${AXON_LLVM_BACKEND}")
message(STATUS "  Install prefix:${CMAKE_INSTALL_PREFIX}")
