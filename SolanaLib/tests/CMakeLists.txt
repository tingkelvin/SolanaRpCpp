# SolanaCpplib/tests/CMakeLists.txt

# Download Catch2 if not already installed (you can switch to FetchContent if you prefer)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

FetchContent_MakeAvailable(googletest)
include(GoogleTest)

foreach(X IN ITEMS EncodingTests CryptoTests NetworkTests)
    add_executable(${X} ${X}.cpp)
    target_link_libraries(${X} 
        GTest::gtest_main 
        SolanaLib
        OpenSSL::SSL
        OpenSSL::Crypto
    )
    gtest_discover_tests(${X})
endforeach()