cmake_minimum_required(VERSION 3.24)
project(SolanaLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.15.3
)
FetchContent_MakeAvailable(spdlog)  # <-- Must come before linking

# Include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/SolanaLib/include)

# Gather sources
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/SolanaLib/src/*.cpp)
file(GLOB_RECURSE LIB_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/SolanaLib/include/*.hpp)

# Build the static library
add_library(SolanaLib STATIC ${LIB_SOURCES} ${LIB_HEADERS})

# Find and link dependencies
find_package(Boost REQUIRED COMPONENTS url system thread headers coroutine json)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)


target_link_libraries(SolanaLib PRIVATE
    spdlog::spdlog
    Boost::url
    Boost::system
    Boost::thread
    Boost::coroutine
    Boost::json
    OpenSSL::Crypto
    OpenSSL::SSL
    nlohmann_json
    pthread
)

target_link_libraries(SolanaLib PUBLIC
    spdlog::spdlog
)

target_include_directories(SolanaLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/SolanaLib/include
    ${Boost_INCLUDE_DIRS}
)

# Build the main app
add_executable(app main.cpp)
target_link_libraries(app PRIVATE SolanaLib)

# Enable tests
enable_testing()
add_subdirectory(SolanaLib/tests)
