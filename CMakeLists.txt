cmake_minimum_required(VERSION 4.0)
project(PhosphorusGUI VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER "C:/ProgramData/mingw64/mingw64/bin/gcc.exe")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(BUILD_SHARED_LIBS "Build using a shared library." ON)

set(LIB_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/phosphorus_gui.c")

add_library(phosphorus_gui ${LIB_SRC})

target_compile_definitions(phosphorus_gui PRIVATE $<$<BOOL:${BUILD_SHARED_LIBS}>:PHOS_GUI_EXPORTS>)

target_include_directories(phosphorus_gui PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(phosphorus_gui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/libtimey.dll.a" "${CMAKE_CURRENT_SOURCE_DIR}/lib/libvibrant_logs.dll.a")

add_executable(phos_gui_test "${CMAKE_CURRENT_SOURCE_DIR}/example.c")
target_link_libraries(phos_gui_test PRIVATE phosphorus_gui)

install(TARGETS phosphorus_gui
	EXPORT phosphorus_gui_targets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT phosphorus_gui_targets
	FILE phosphorus_gui_targets.cmake
	NAMESPACE PhosphorusGUI::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PhosphorusGUI)

configure_package_config_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/phosphorus_gui_config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/phosphorus_gui_config.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PhosphorusGUI)

write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/phosphorus_gui_configVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/phosphorus_gui_config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/phosphorus_gui_configVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PhosphorusGUI)
