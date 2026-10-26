cmake_minimum_required(VERSION 3.16)
project(Calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Результат сборки лежит в build/bin/Debug/ (или Release): там calc.exe и plugins/.

set(CALC_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

# Флаги предупреждений. Для MSVC: /W4 — показывать почти все предупреждения,
# /utf-8 — чтобы русские комментарии не ломались, /EHsc — включить исключения C++.
function(calc_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf-8 /EHsc)
    else()
        # Для MinGW: -static компилирует библиотеки внутрь, рядом с calc.exe
      
        target_compile_options(${target} PRIVATE -Wall -Wextra)
        target_link_options(${target} PRIVATE -static)
    endif()
endfunction()

# Общий заголовок plugin_api.h.
add_library(calc_plugin_api INTERFACE)
target_include_directories(calc_plugin_api INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 
add_executable(calc
    src/main.cpp
    src/OperationRegistry.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/DynamicLibrary.cpp
    src/PluginLoader.cpp)
target_link_libraries(calc PRIVATE calc_plugin_api)
set_target_properties(calc PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CALC_OUTPUT_DIR})
calc_set_warnings(calc)

# Плагины 
add_subdirectory(plugins)

# Чтобы при сборке в IDE собирались и плагины, а не только calc.
add_dependencies(calc ${CALC_PLUGINS})
