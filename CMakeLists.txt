cmake_minimum_required(VERSION 3.20)
project(2DGameEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_compile_definitions(
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Quellen: src/ + items/ + buildings/ + dimensions/ + ui/ + npcs/ + npc_config/ ──
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/items/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/items/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/items/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/buildings/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/buildings/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/buildings/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/dimensions/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dimensions/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/dimensions/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/player/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/player/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/player/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/npcs/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/npcs/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/npcs/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/objects/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/objects/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/objects/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/hud/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/hud/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/hud/*.cxx"
)

message(STATUS "Gefundene Quell-Dateien:")
foreach(SOURCE_FILE ${SOURCES})
    message(STATUS "  - ${SOURCE_FILE}")
endforeach()

# ── FetchContent ──────────────────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.0
    GIT_SHALLOW    TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(rlimgui)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# ── ImGui Library ─────────────────────────────────────────────────────────────
add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
)
target_link_libraries(imgui_lib PUBLIC raylib)

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/items
    ${CMAKE_SOURCE_DIR}/buildings
    ${CMAKE_SOURCE_DIR}/dimensions
    ${CMAKE_SOURCE_DIR}/ui
    ${CMAKE_SOURCE_DIR}/player
    ${CMAKE_SOURCE_DIR}/npcs
    ${CMAKE_SOURCE_DIR}/objects
    ${CMAKE_SOURCE_DIR}/hud
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
)

# ── ASSETS_PATH: immer auf den assets/-Ordner im Projektverzeichnis zeigen ───
# Egal ob Debug/Release/aus dem Build-Ordner gestartet – der Pfad stimmt immer.
# Für einen echten Release-Build diesen Define einfach weglassen und die
# Assets neben die .exe kopieren (Fallback "assets" greift dann).
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ASSETS_PATH="${CMAKE_SOURCE_DIR}/assets"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    raylib
    imgui_lib
    nlohmann_json::nlohmann_json
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        NOGDI
        VC_EXTRALEAN
    )
elseif(APPLE)
    find_library(COCOA_LIBRARY    Cocoa)
    find_library(IOKIT_LIBRARY    IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY}
    )
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        GL m pthread dl rt X11
    )
endif()

message(STATUS "=== 2DGameEngine Konfiguration ===")
message(STATUS "Build-Dir:   ${CMAKE_BINARY_DIR}")
message(STATUS "Bin-Dir:     ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "Assets-Dir:  ${CMAKE_SOURCE_DIR}/assets")

# ── Reset-World Tool ──────────────────────────────────────────────────────────
add_executable(reset_world "${CMAKE_SOURCE_DIR}/tools/reset_world.cpp")
set_target_properties(reset_world PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
)
target_compile_features(reset_world PRIVATE cxx_std_17)
target_link_libraries(reset_world PRIVATE nlohmann_json::nlohmann_json)
target_compile_definitions(reset_world PRIVATE
    ASSETS_PATH="${CMAKE_SOURCE_DIR}/assets"
)
if(WIN32)
    target_compile_definitions(reset_world PRIVATE
        WIN32_LEAN_AND_MEAN NOMINMAX _CRT_SECURE_NO_WARNINGS
    )
endif()