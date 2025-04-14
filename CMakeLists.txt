cmake_minimum_required(VERSION 3.24)
project(SolanaLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include header files for the Solana library
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/SolanaLib/include)

# Gather sources and headers for the SolanaLib
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/SolanaLib/src/*.cpp)

file(GLOB_RECURSE LIB_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/SolanaLib/include/*.hpp)

# Build the static library (SolanaLib)
add_library(SolanaLib STATIC ${LIB_SOURCES} ${LIB_HEADERS})

# Specify public include directories for consumers of SolanaLib
target_include_directories(SolanaLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/SolanaLib/include
)

# Find Boost and specify components required
find_package(Boost REQUIRED COMPONENTS
    url
    system
    thread
    headers
    coroutine
)

# Link Boost libraries to the main project (app)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::url
    Boost::system
    Boost::thread
    Boost::coroutine
)

# Ensure Boost includes are available to the SolanaLib
target_include_directories(SolanaLib PUBLIC
    ${Boost_INCLUDE_DIRS}
)

find_package(OpenSSL REQUIRED)

target_link_libraries(SolanaLib PRIVATE OpenSSL::Crypto)


# Build the main executable (app)
add_executable(app main.cpp)

# Link SolanaLib to the app executable
target_link_libraries(app PRIVATE SolanaLib)

# Enable testing for the project
enable_testing()
add_subdirectory(SolanaLib/tests)
