
file(GLOB imgui_SRC_GLOB
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/cpp/*.cpp"
)

set(IMGUI_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp
        ${imgui_SRC_GLOB}
)

# This is a fixed shaderc lib
add_library(custom_shaderc_combined STATIC IMPORTED GLOBAL)
set_property(TARGET custom_shaderc_combined APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(custom_shaderc_combined PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
        IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/shaderc/libshaderc_combined.a"
)


target_include_directories(terrain_renderer
        PUBLIC
            imgui
            imgui/backends
            imgui/misc/cpp

            stb_image

            custom_shaderc_combined
)

target_sources(terrain_renderer
    PUBLIC
        ${IMGUI_SOURCES}
)
