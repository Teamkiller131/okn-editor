cmake_minimum_required(VERSION 3.20)
project(okn-editor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# okn-editor is a Dear ImGui editor built on TeamkillerUniGUI (third_party/), which
# replaces the old Qt6/QML shell. It edits REAL engine state: the "scene" is a
# SliceWorld/LuaSlice (okn-ecs + okn-physics + the sprite path), authored in Lua.
# Builds only when unigui::unigui is available (i.e. inside the OmniKillerNexus root
# build); otherwise a stub lib keeps SDK references resolving.

if(TARGET unigui::unigui)
    find_package(Lua QUIET)
    find_package(sol2 CONFIG QUIET)

    set(EDITOR_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/editor.cpp")
    set(EDITOR_PUBLIC_LIBS okn-core okn-math)
    set(EDITOR_PRIVATE_INC "")
    set(EDITOR_HAS_SCENE OFF)

    set(RENDER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../okn-render")
    if(sol2_FOUND AND LUA_FOUND AND EXISTS "${RENDER_DIR}/slice/lua_slice.cpp")
        # Compile the Lua scene bridge directly (header-only slice + lua_slice.cpp).
        list(APPEND EDITOR_SRC "${RENDER_DIR}/slice/lua_slice.cpp")
        list(APPEND EDITOR_PUBLIC_LIBS okn-ecs okn-physics sol2::sol2 ${LUA_LIBRARIES})
        list(APPEND EDITOR_PRIVATE_INC "${RENDER_DIR}/include" "${LUA_INCLUDE_DIR}")
        set(EDITOR_HAS_SCENE ON)
    endif()

    add_library(okn-editor STATIC ${EDITOR_SRC})
    target_include_directories(okn-editor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_include_directories(okn-editor PRIVATE ${EDITOR_PRIVATE_INC})
    target_link_libraries(okn-editor PUBLIC ${EDITOR_PUBLIC_LIBS})
    target_link_libraries(okn-editor PRIVATE unigui::unigui)
    target_compile_definitions(okn-editor PUBLIC OKN_EDITOR_HAS_UNIGUI=1)
    if(EDITOR_HAS_SCENE)
        target_compile_definitions(okn-editor PRIVATE OKN_EDITOR_HAS_SCENE=1)
    endif()
    if(MSVC)
        target_compile_options(okn-editor PRIVATE /WX-)   # imgui/unigui/sol2 interplay
    endif()

    add_executable(okn-editor-app "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    target_link_libraries(okn-editor-app PRIVATE okn-editor unigui::unigui)
    target_compile_features(okn-editor-app PRIVATE cxx_std_23)
    if(MSVC)
        target_compile_options(okn-editor-app PRIVATE /WX-)
    endif()

    if(EDITOR_HAS_SCENE AND EXISTS "${RENDER_DIR}/slice/slice_scene.lua")
        add_custom_command(TARGET okn-editor-app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${RENDER_DIR}/slice/slice_scene.lua"
                "$<TARGET_FILE_DIR:okn-editor-app>/slice_scene.lua")
    endif()
else()
    add_library(okn-editor STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_stub.cpp")
    target_include_directories(okn-editor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(okn-editor PUBLIC okn-core okn-math)
    message(STATUS "okn-editor: unigui::unigui not found — stub lib only")
endif()
