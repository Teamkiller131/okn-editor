cmake_minimum_required(VERSION 3.20)
project(okn-editor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# okn-editor is a Dear ImGui editor built on TeamkillerUniGUI (third_party/), which
# replaces the old Qt6/QML shell. The editor target only builds when unigui::unigui
# is available (i.e. inside the OmniKillerNexus root build); otherwise a stub lib is
# produced so SDK references still resolve.

if(TARGET unigui::unigui)
    add_library(okn-editor STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/editor.cpp")
    target_include_directories(okn-editor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(okn-editor PUBLIC okn-core okn-math)
    target_link_libraries(okn-editor PRIVATE unigui::unigui)
    target_compile_definitions(okn-editor PUBLIC OKN_EDITOR_HAS_UNIGUI=1)
    if(MSVC)
        target_compile_options(okn-editor PRIVATE /WX-)   # imgui/unigui interplay
    endif()

    add_executable(okn-editor-app "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    target_link_libraries(okn-editor-app PRIVATE okn-editor unigui::unigui)
    target_compile_features(okn-editor-app PRIVATE cxx_std_23)
    if(MSVC)
        target_compile_options(okn-editor-app PRIVATE /WX-)
    endif()
else()
    add_library(okn-editor STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_stub.cpp")
    target_include_directories(okn-editor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(okn-editor PUBLIC okn-core okn-math)
    message(STATUS "okn-editor: unigui::unigui not found — stub lib only")
endif()
