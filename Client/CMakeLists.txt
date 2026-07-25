cmake_minimum_required(VERSION 3.15)
project(SonyCompanionApp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# ── Executable ───────────────────────────────────────────────────────────────
add_executable(SonyCompanionApp WIN32)

add_definitions(-D__HEADPHONES_APP_VERSION__="1.0.0")

# ── Core Protocol Sources ────────────────────────────────────────────────────
target_sources(SonyCompanionApp PRIVATE
    BluetoothWrapper.cpp
    ByteMagic.cpp
    CommandSerializer.cpp
    TimedMessageQueue.cpp
    CrossPlatformGUI.cpp
    Headphones.cpp
    CascadiaCodeFont.cpp
    MediaManager.cpp
)

target_include_directories(SonyCompanionApp PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/imgui
)

# ── Dear ImGui Core ──────────────────────────────────────────────────────────
target_sources(SonyCompanionApp PRIVATE
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
)

# ── Windows Platform Sources ─────────────────────────────────────────────────
target_sources(SonyCompanionApp PRIVATE
    windows/main.cpp
    windows/WindowsGUI.cpp
    windows/WindowsBluetoothConnector.cpp
    windows/app.rc
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_dx11.cpp
)

# ── Windows Libraries ────────────────────────────────────────────────────────
target_link_libraries(SonyCompanionApp PRIVATE
    Threads::Threads
    ws2_32
    rpcrt4
    Bthprops
    d3d11
    dxgi
    dwmapi
)

# ── Copy resources to build directory ────────────────────────────────────────
add_custom_command(TARGET SonyCompanionApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:SonyCompanionApp>/resources"
    COMMENT "Copying resources..."
)

# ── Set subsystem to WINDOWS (no console flash) ─────────────────────────────
set_target_properties(SonyCompanionApp PROPERTIES
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "SonyCompanionApp"
)

# For MSVC — suppress common warnings on third-party code
if(MSVC)
    target_compile_options(SonyCompanionApp PRIVATE /W3 /wd4996 /wd4267 /wd4244)
    # Link-time optimization
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
endif()
