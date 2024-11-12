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
)

target_include_directories(terrain_renderer
    PRIVATE .
    PUBLIC ${VULKAN_INCLUDE_DIRS}
)

include(src/CMakeLists.txt)