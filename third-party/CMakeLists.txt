
file(GLOB imgui_SRC_GLOB
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/cpp/*.cpp"
)

set(IMGUI_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp
        ${imgui_SRC_GLOB}
)

target_include_directories(terrain_renderer
        PUBLIC
            imgui
            imgui/backends
            imgui/misc/cpp

            stb_image
)

target_sources(terrain_renderer
    PUBLIC
        ${IMGUI_SOURCES}
)