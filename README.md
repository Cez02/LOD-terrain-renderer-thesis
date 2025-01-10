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

In order to use the app, traverse into the built binary directory. Next create a `heightmaps` folder like the one in the root directory of the repo and copy desired SRTM heightmaps inside. An example heightmap is also included in the `heightmaps` directory. The program will load the heightmaps inside the directory - note this is not a recursive action so subdirectories won't be searched. Other SRTM heightmaps can be found at the [coverage map site](https://www.viewfinderpanoramas.org/Coverage%20map%20viewfinderpanoramas_org3.htm).

Currently there is no way to set the observer's geographic coordinates. This feature will be added a command-line option.


## Structure

A mesh shader workgroup consists of 64 invocations. Such workgroup is capable of rendering:
1. 1 full-resolution meshlet
2. 4 LOD-2 meshlets
3. 16 LOD-4 meshlets
4. 64 LOD-8 meshlets

Since this is the case, the natural proposition is to pack the meshlets into MeshShaderProcessingUnit structs.

```
MeshShaderProcessingUnit {
    uint8_t meshletOffsetsData
    uint8_t meshletCount;
}
```

This will dictate how many meshlets the mesh shader is responsible for drawing.

The task shader payload will in turn look like this:

```
MeshShaderProcessingUnit meshShaderProcessingUnits[SIZE_OF_TASK_WORKGROUP * MESHLETS_PER_TASK_INVOCATION];
u8vec2 meshletDeltas[SIZE_OF_TASK_WORKGROUP * MESHLETS_PER_TASK_INVOCATION]; // contains (delta, LODLevel)
```


The task shader will do the following:

- Each invocation iterates over k meshlets
    - take meshlet, decide the LOD, register the meshlet in meshletDeltas

- at the end, if invocation == 0
    - sort the meshletDeltas by decreasing LODLevel
    - Create meshShaderProcessingUnits that abide to the capabilities of the mesh shader workgroups
    - emit as many mesh shaders as there are meshShaderProcessingUnits