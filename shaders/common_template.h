#ifndef LOD_TERRAIN_RENDERING_THESIS_PROJECT_COMMON_H
#define LOD_TERRAIN_RENDERING_THESIS_PROJECT_COMMON_H

#define UNSET 1

#define MAX_PREFERRED_MESH_WORK_GROUP_INVOCATIONS $m_MaxPreferredMeshWorkGroupInvocations
#define MAX_PREFERRED_TASK_WORK_GROUP_INVOCATIONS $m_MaxPreferredTaskWorkGroupInvocations
#define MAX_MESH_OUTPUT_VERTICES $m_MaxMeshOutputVertices
#define MAX_MESH_OUTPUT_PRIMITIVES $m_MaxMeshOutputPrimitives


#ifdef __cplusplus
#include <glm/glm.hpp>

using namespace glm;
#endif

#define MESHLET_LENGTH $meshletLength

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
