cmake_minimum_required(VERSION 3.20)
project(okn-editor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_SAMPLES "Build samples" ON)
option(BUILD_TESTS   "Build tests"   ON)

option(OKN_EDITOR_ENABLE_PLUGINS       "Enable plugin system" ON)
option(OKN_EDITOR_ENABLE_GIT           "Enable Git integration" ON)
option(OKN_EDITOR_ENABLE_I18N          "Enable I18N" ON)
option(OKN_EDITOR_ENABLE_PROFILING     "Enable profiling panels" ON)
option(OKN_EDITOR_ENABLE_FRAME_DEBUG   "Enable frame debugger panel" ON)
option(OKN_EDITOR_ENABLE_NET           "Enable network panels" ON)
option(OKN_EDITOR_ENABLE_SCRIPT_DEBUG  "Enable script debugger" ON)
option(OKN_EDITOR_ENABLE_AUTOSAVE      "Enable autosave/recovery" ON)
option(OKN_EDITOR_ENABLE_CLI_TOOLS     "Enable CLI integration" ON)

file(GLOB_RECURSE okn-editor_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(okn-editor STATIC ${okn-editor_SRC})
target_include_directories(okn-editor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

foreach(opt IN ITEMS
        OKN_EDITOR_ENABLE_PLUGINS
        OKN_EDITOR_ENABLE_GIT
        OKN_EDITOR_ENABLE_I18N
        OKN_EDITOR_ENABLE_PROFILING
        OKN_EDITOR_ENABLE_FRAME_DEBUG
        OKN_EDITOR_ENABLE_NET
        OKN_EDITOR_ENABLE_SCRIPT_DEBUG
        OKN_EDITOR_ENABLE_AUTOSAVE
        OKN_EDITOR_ENABLE_CLI_TOOLS)
    if(${opt})
        target_compile_definitions(okn-editor PUBLIC ${opt}=1)
    endif()
endforeach()

# Editor executable target
add_executable(okn-editor-app "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(okn-editor-app PRIVATE okn-editor)
target_compile_features(okn-editor-app PRIVATE cxx_std_23)

if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()