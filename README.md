# lod-terrain-renderer-thesis
LOD terrain renderer using mesh shaders via Vulkan (thesis project)


## Building

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
cmake -DCMAKE_BUILD_TYPE=<BUILD_TYPE> --preset <BUILD_TYPE> -S . -B ./build/Debug
```
