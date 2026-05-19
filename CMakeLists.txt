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

set(CMAKE_AUTORCC ON)

# Qt6 detection
find_package(Qt6 QUIET COMPONENTS Core Gui Quick QuickControls2)
if(Qt6_FOUND)
    set(OKN_HAS_QT TRUE)
    message(STATUS "okn-editor: Qt6 found — building QML editor")
else()
    set(OKN_HAS_QT FALSE)
    message(STATUS "okn-editor: Qt6 not found — building headless editor library only")
endif()

file(GLOB_RECURSE ${PROJECT_NAME}_SRC
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Exclude Qt-only source files from the base list
list(FILTER ${PROJECT_NAME}_SRC EXCLUDE REGEX "editor_app_qt\\.cpp$")

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4100)
endif()

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

if(OKN_HAS_QT)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OKN_HAS_QT=1)
    set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC ON)

    # Add Qt-dependent source files
    target_sources(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_app_qt.cpp"
    )

    target_link_libraries(${PROJECT_NAME} PUBLIC
        Qt6::Core Qt6::Gui Qt6::Quick Qt6::QuickControls2
    )

    set(EDITOR_APP_TARGET okn-editor-app)
    add_executable(${EDITOR_APP_TARGET}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/qml.qrc"
    )
    set_target_properties(${EDITOR_APP_TARGET} PROPERTIES AUTOMOC ON AUTORCC ON)
    target_link_libraries(${EDITOR_APP_TARGET} PRIVATE
        ${PROJECT_NAME}
        Qt6::Core Qt6::Gui Qt6::Quick Qt6::QuickControls2
    )
    target_include_directories(${EDITOR_APP_TARGET} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )
    target_compile_features(${EDITOR_APP_TARGET} PRIVATE cxx_std_23)
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
