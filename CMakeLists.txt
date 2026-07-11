cmake_minimum_required(VERSION 4.0)
project(PhosphorusGUI VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER "C:/ProgramData/mingw64/mingw64/bin/gcc.exe")

option(BUILD_SHARED_LIBS "Build using a shared library." ON)

add_library(phosphorus_gui "${CMAKE_CURRENT_SOURCE_DIR}/src/phosphorus_gui.c")

target_compile_definitions(phosphorus_gui PRIVATE $<$<BOOL:${BUILD_SHARED_LIBS}>:PHOS_GUI_EXPORTS>)

target_include_directories(phosphorus_gui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(phosphorus_gui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/libtimey.dll.a" "${CMAKE_CURRENT_SOURCE_DIR}/lib/libvibrant_logs.dll.a"
											"${CMAKE_CURRENT_SOURCE_DIR}/lib/libraylibdll.a")

add_executable(phos_gui_test "${CMAKE_CURRENT_SOURCE_DIR}/example.c")
target_link_libraries(phos_gui_test PRIVATE phosphorus_gui)
