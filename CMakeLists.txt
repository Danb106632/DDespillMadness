cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(D_DespillMadness)

set(CMAKE_MODULE_PATH "CMake;${CMAKE_MODULE_PATH}")
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

if (MSVC)
    add_compile_definitions(HAVE_SNPRINTF NOMINMAX _USE_MATH_DEFINES)
endif()

find_package(Nuke REQUIRED)


function(add_example_plugin PLUGIN_NAME)
    add_nuke_plugin(${PLUGIN_NAME} ${ARGN})
    target_compile_definitions(${PLUGIN_NAME} PRIVATE FN_EXAMPLE_PLUGIN)
endfunction()

add_example_plugin(D_DespillMadness D_DespillMadness.cpp)
