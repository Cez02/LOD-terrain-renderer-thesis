#ifndef LOD_TERRAIN_RENDERING_THESIS_PROJECT_COMMON_H
#define LOD_TERRAIN_RENDERING_THESIS_PROJECT_COMMON_H

/*
 *
        uint32_t m_MaxMeshOutputVertices;
        uint32_t m_MaxMeshOutputPrimitives;
        uint32_t m_MaxPreferredTaskWorkGroupInvocations;
        uint32_t m_MaxPreferredMeshWorkGroupInvocations;

 */

#define UNSET 1

#define CMN_MAX_PREFERRED_MESH_WORK_GROUP_INVOCATIONS 32
#define CMN_MAX_PREFERRED_TASK_WORK_GROUP_INVOCATIONS 32
#define CMN_MAX_MESH_OUTPUT_VERTICES 256
#define CMN_MAX_MESH_OUTPUT_PRIMITIVES 256

#define CMN_MAX_TASK_WORKGROUP_SIZE 128
#define CMN_MAX_MESH_WORKGROUP_SIZE 128


#ifdef __cplusplus
#include <glm/glm.hpp>

using namespace glm;
#endif

#define CMN_MAX_MESHLET_DIMENSION_LENGTH 16


struct
#ifdef __cplusplus
alignas(16)
#endif
MeshletDescription {
#ifdef __cplusplus
    alignas(8)
#endif
    uvec2 Offset; // starting point of the meshlet in the chunk

#ifdef __cplusplus
    alignas(8)
#endif
    uvec2 Dimensions; // dimensions of the meshlet

    uint HeightmapDataOffset;
};


#endif //LOD_TERRAIN_RENDERING_THESIS_PROJECT_COMMON_H
