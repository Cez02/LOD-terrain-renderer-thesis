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
#define CMN_MAX_MESH_OUTPUT_VERTICES 256
#define CMN_MAX_MESH_OUTPUT_PRIMITIVES 256

#ifdef __cplusplus
#include <glm/glm.hpp>

using namespace glm;
#endif

//struct MeshletDescription {
//    u8vec2 Offset; // starting point of the meshlet in the chunk
//    u8vec2 Dimensions; // dimensions of the meshlet
//};

#endif //LOD_TERRAIN_RENDERING_THESIS_PROJECT_COMMON_H
