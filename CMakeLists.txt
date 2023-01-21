cmake_minimum_required(VERSION 3.6)

project(
    wf-prometheus
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
set(CMAKE_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

find_library(LIBRT rt)
find_package(OpenSSL REQUIRED)

if (Workflow_DIR)
    find_package(Workflow REQUIRED CONFIG HINTS ${Workflow_DIR})
else ()
    find_package(Workflow REQUIRED CONFIG)
endif ()

include_directories(${OPENSSL_INCLUDE_DIR} ${WORKFLOW_INCLUDE_DIR} ${INC_DIR})
link_directories(${WORKFLOW_LIB_DIR})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})

set(INCLUDE_HEADERS
	src/Var.h
	src/GaugeVar.h
	src/CounterVar.h
	src/HistogramVar.h
	src/TimeWindowQuantiles.h
	src/CKMSQuantiles.h
	src/SummaryVar.h
	src/Exporter.h
	src/VarFactory.h
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

### CONFIG

include(CMakePackageConfigHelpers)
set(CONFIG_INC_DIR ${INC_DIR})
set(CONFIG_LIB_DIR ${LIB_DIR})
configure_package_config_file(
	${PROJECT_NAME}-config.cmake.in
	${PROJECT_SOURCE_DIR}/${PROJECT_NAME}-config.cmake
	INSTALL_DESTINATION ${CMAKE_CONFIG_INSTALL_DIR}
	PATH_VARS CONFIG_INC_DIR CONFIG_LIB_DIR
)

set(CMAKE_CONFIG_INSTALL_FILE ${PROJECT_BINARY_DIR}/config.toinstall.cmake)
set(CONFIG_INC_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(CONFIG_LIB_DIR ${CMAKE_INSTALL_LIBDIR})
configure_package_config_file(
	${PROJECT_NAME}-config.cmake.in
	${CMAKE_CONFIG_INSTALL_FILE}
	INSTALL_DESTINATION ${CMAKE_CONFIG_INSTALL_DIR}
	PATH_VARS CONFIG_INC_DIR CONFIG_LIB_DIR
)

### INSTALL

install (
    FILES ${INCLUDE_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    COMPONENT devel
)

### EXAMPLE

add_executable(example example.cc)

target_link_libraries(example ${PROJECT_NAME} workflow pthread ssl crypto)

