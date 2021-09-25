cmake_minimum_required(VERSION 3.6)

find_library(LIBRT rt)
find_package(OpenSSL REQUIRED)
find_package(workflow REQUIRED)

set(SRC
	Var.cc
	Exporter.cc
	VarFactory.cc
)

add_library(${PROJECT_NAME} STATIC ${SRC})

install(
    TARGETS ${PROJECT_NAME}
    ARCHIVE
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT devel
)

