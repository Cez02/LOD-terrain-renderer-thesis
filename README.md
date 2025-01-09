# lod-terrain-renderer-thesis
LOD terrain renderer using mesh shaders via Vulkan (thesis project)


## Building

### Cloning

When cloning the repo, remember to use the `--recurse-submodules` as imgui is also cloned as a git submodule.

### Dependencies

In order to build the project, the following are required:
- Support for vulkan 1.3 and vulkan-sdk
- GLFW 3
- GLM


### Compiling

In order to build the project execute the following commands:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=<BUILD_TYPE> --preset <BUILD_TYPE> -S . -B ./build/<BUILD_TYPE>
cmake --build --target terrain_renderer --preset <BUILD_TYPE>
```

### Options

Additional options can be added when calling the first `cmake` command by adding `-D<Option name>=ON`:
- `DRAW_WIREFRAME` - draws the terrain in wireframe.


## Usage

In order to use the app, traverse into the built binary directory. Next create a `heightmaps` folder like the one in the root directory of the repo and copy desired SRTM heightmaps inside. An example heightmap is also included in the `heightmaps` directory. The program will load the heightmaps inside the directory - note this is not a recursive action so subdirectories won't be searched.

Currently there is no way to set the observer's geographic coordinates. This feature will be added a command-line option.
