cmake_minimum_required(VERSION 3.6)

project(
    workflow-prometheus
    VERSION 1.0
    LANGUAGES CXX
)

include(GNUInstallDirs)

set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "build type")
set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -fPIC -pipe -std=c++11 -fno-exceptions")

set(INC_DIR ${PROJECT_SOURCE_DIR}/_include CACHE PATH "workflow inc")
set(LIB_DIR ${PROJECT_SOURCE_DIR}/_lib CACHE PATH "workflow lib")

include_directories(${OPENSSL_INCLUDE_DIR} ${WORKFLOW_INCLUDE_DIR} ${INC_DIR})
link_directories(${WORKFLOW_LIB_DIR})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})

set(INCLUDE_HEADERS
    src/Vars.h
    src/GaugeVars.h
    src/CounterVars.h
    src/Exporter.h
)

add_custom_target(
	LINK_HEADERS ALL
	COMMENT "link headers..."
)

macro(makeLink src dest target)
    add_custom_command(
        TARGET ${target} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dest}
        DEPENDS ${dest}
    )
endmacro()

add_custom_command(
    TARGET LINK_HEADERS PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${INC_DIR}/${PROJECT_NAME}
)

foreach(header_file ${INCLUDE_HEADERS})
    string(REPLACE "/" ";" arr ${header_file})
    list(GET arr -1 file_name)
    makeLink(${PROJECT_SOURCE_DIR}/${header_file} ${INC_DIR}/${PROJECT_NAME}/${file_name} LINK_HEADERS)
endforeach()

add_subdirectory(src)

install (
    FILES ${INCLUDE_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    COMPONENT devel
)

add_executable(example example.cc)
find_library(LIBRT rt)
find_package(OpenSSL REQUIRED)
find_package(workflow REQUIRED)
target_link_libraries(example ${PROJECT_NAME} workflow)

