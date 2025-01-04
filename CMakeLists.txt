cmake_minimum_required(VERSION 3.12)
project("LOD terrain rendering thesis project")

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_executable(terrain_renderer)

# dependencies

add_subdirectory(third-party)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined SPIRV-Tools glslang glslangValidator)
find_package(glm REQUIRED)

set(SHADERC_LIBS
        shaderc_combined
)

include(shaders/CMakeLists.txt)

target_compile_definitions(terrain_renderer
    PUBLIC
        GLFW_INCLUDE_VULKAN
)

target_link_libraries(terrain_renderer
    PUBLIC
        glfw
        ${SHADERC_LIBS}
        ${Vulkan_LIBRARIES}
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