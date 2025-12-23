cmake_minimum_required(VERSION 3.20)
project(okn-editor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_SAMPLES "Build samples" ON)
option(BUILD_TESTS   "Build tests"   ON)

# Feature toggles
option(OKN_EDITOR_ENABLE_PLUGINS       "Enable plugin system" ON)
option(OKN_EDITOR_ENABLE_GIT           "Enable Git integration" ON)
option(OKN_EDITOR_ENABLE_I18N          "Enable I18N" ON)
option(OKN_EDITOR_ENABLE_PROFILING     "Enable profiling panels" ON)
option(OKN_EDITOR_ENABLE_FRAME_DEBUG   "Enable frame debugger panel" ON)
option(OKN_EDITOR_ENABLE_NET           "Enable network panels" ON)
option(OKN_EDITOR_ENABLE_SCRIPT_DEBUG  "Enable script debugger" ON)
option(OKN_EDITOR_ENABLE_AUTOSAVE      "Enable autosave/recovery" ON)
option(OKN_EDITOR_ENABLE_CLI_TOOLS     "Enable CLI tool integration" ON)

file(GLOB_RECURSE ${PROJECT_NAME}_SRC
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Compile definitions
if(OKN_EDITOR_ENABLE_PLUGINS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OKN_EDITOR_ENABLE_PLUGINS=1)
endif()
if(OKN_EDITOR_ENABLE_GIT)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OKN_EDITOR_ENABLE_GIT=1)
endif()
if(OKN_EDITOR_ENABLE_I18N)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OKN_EDITOR_ENABLE_I18N=1)
endif()
if(OKN_EDITOR_ENABLE_PROFILING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OKN_EDITOR_ENABLE_PROFILING=1)
endif()
if(OKN_EDITOR_ENABLE_FRAME_DEBUG)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OKN_EDITOR_ENABLE_FRAME_DEBUG=1)
endif()
if(OKN_EDITOR_ENABLE_NET)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OKN_EDITOR_ENABLE_NET=1)
endif()
if(OKN_EDITOR_ENABLE_SCRIPT_DEBUG)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OKN_EDITOR_ENABLE_SCRIPT_DEBUG=1)
endif()
if(OKN_EDITOR_ENABLE_AUTOSAVE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OKN_EDITOR_ENABLE_AUTOSAVE=1)
endif()
if(OKN_EDITOR_ENABLE_CLI_TOOLS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OKN_EDITOR_ENABLE_CLI_TOOLS=1)
endif()

# Samples
if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()