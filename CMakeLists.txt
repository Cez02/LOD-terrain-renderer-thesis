cmake_minimum_required(VERSION 3.12)
project("LOD terrain rendering thesis project")


add_executable(terrain_renderer)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)

include(shaders/CMakeLists.txt)

target_compile_definitions(terrain_renderer
    PUBLIC
        GLFW_INCLUDE_VULKAN
)

target_link_libraries(terrain_renderer
    PUBLIC
        glfw
        vulkan
        glm::glm
        shaderc_combined
)

target_include_directories(terrain_renderer
    PRIVATE .
    PUBLIC ${VULKAN_INCLUDE_DIRS}
)

target_compile_definitions(terrain_renderer
    PUBLIC
        SHADER_SOURCES_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        SHADER_BINARIES_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/shaders"
)

include(src/CMakeLists.txt)