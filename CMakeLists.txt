cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(DDespillMadness)

set(CMAKE_MODULE_PATH "CMake;${CMAKE_MODULE_PATH}")
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

if (MSVC)
    add_compile_definitions(HAVE_SNPRINTF NOMINMAX _USE_MATH_DEFINES)
endif()

find_package(Nuke REQUIRED)

# cxx standards
if (NUKE_VERSION_MAJOR VERSION_GREATER_EQUAL 15)
    # Nuke 15.x: C++17 + new ABI
    message("Configuring for Nuke 15.x: C++17")
    # https://learn.foundry.com/nuke/developers/150/ndkdevguide/appendixa/linux.html
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=1")
    set(CMAKE_CXX_STANDARD 17)
elseif (NUKE_VERSION_MAJOR VERSION_GREATER_EQUAL 14)
    # Nuke 14.x: C++17 + old ABI
    message("Configuring for Nuke 14.x: C++17") 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
    set(CMAKE_CXX_STANDARD 17)
elseif (NUKE_VERSION_MAJOR VERSION_GREATER_EQUAL 13)
    # Nuke 13.x: C++14
    message("Configuring for Nuke 13.x: C++14")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
    set(CMAKE_CXX_STANDARD 14)
else()
    # Nuke 12.x: C++11
    message("Configuring for Nuke 12.x: C++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
    set(CMAKE_CXX_STANDARD 11)
endif()

function(add_example_plugin PLUGIN_NAME)
    add_nuke_plugin(${PLUGIN_NAME} ${ARGN})
    target_compile_definitions(${PLUGIN_NAME} PRIVATE FN_EXAMPLE_PLUGIN)
endfunction()

add_example_plugin(DDespillMadness DDespillMadness.cpp)
