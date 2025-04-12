# Top-level CMakeLists.txt

cmake_minimum_required(VERSION 3.24)

project(SolanaRpCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
# FetchContent_MakeAvailable(json)

# find_package(OpenSSL REQUIRED)


add_library(Borsh INTERFACE)
target_include_directories(Borsh INTERFACE ${borsh_cpp_SOURCE_DIR}/borsh-cpp/src)


set(LIBRARY_NAME solana_rpcpp)

if (UNIX)
    add_compile_options("-fexceptions")
endif(UNIX)

# add_subdirectory(Rpc)
# add_subdirectory(Core)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    add_executable(app main.cpp)
    # target_link_libraries(app PRIVATE Solana.Rpc Solana.Core.Programs Solana.Core.Network)
    add_custom_command(TARGET app POST_BUILD
        COMMAND app
        COMMENT "Running app after build"
    )
endif()

# set(BUILD_TEST ON CACHE BOOL "Whether to build tests")
# if (BUILD_TEST)
#     enable_testing()
#     add_subdirectory(test)
# endif()