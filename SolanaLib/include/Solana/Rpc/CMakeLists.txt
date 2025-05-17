
project(Solana.Rpc)

file(
    GLOB_RECURSE
    HEADER_LIST
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)


add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADER_LIST})

target_link_libraries(${PROJECT_NAME} PUBLIC
        Solana.Core.Common
        Solana.Core.Network
        nlohmann_json::nlohmann_json)
target_include_directories(${PROJECT_NAME} PUBLIC include)

source_group(
        TREE "include"
        PREFIX "Header Files"
        FILES ${HEADER_LIST})
