file(GLOB terrain_renderer_SRC
    "src/*.h"
    "src/*.hpp"
    "src/*.cpp"
    "*.h"
    "*.hpp"
    "*.cpp"
)

target_sources(terrain_renderer
    PUBLIC
        ${terrain_renderer_SRC}
)

target_include_directories(terrain_renderer
    PUBLIC .
)