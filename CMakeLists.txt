cmake_minimum_required(VERSION 3.12)
project("LOD terrain rendering thesis project")

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_executable(terrain_renderer)

# options

option(DRAW_WIREFRAME "Draw the terrain wireframe." OFF)

# dependencies

add_subdirectory(third-party)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)

include(shaders/CMakeLists.txt)

target_compile_definitions(terrain_renderer
    PUBLIC
        GLFW_INCLUDE_VULKAN
)

if(DRAW_WIREFRAME)
    target_compile_definitions(terrain_renderer
        PUBLIC
            DRAW_WIREFRAME
    )
endif()

target_link_libraries(terrain_renderer
    PUBLIC
        glfw
        custom_shaderc_combined
        vulkan
)

target_include_directories(terrain_renderer
    PRIVATE .
    PUBLIC ${VULKAN_INCLUDE_DIRS}
)

target_include_directories(terrain_renderer
        PRIVATE
            ./shaders
)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_definitions(terrain_renderer
    PUBLIC
        SHADER_SOURCES_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        SHADER_BINARIES_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/shaders"
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(terrain_renderer
            PUBLIC
            DEBUG
    )
endif()

include(src/CMakeLists.txt)