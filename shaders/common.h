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

#define CMN_MAX_PREFERRED_MESH_WORK_GROUP_INVOCATIONS UNSET
#define CMN_MAX_MESH_OUTPUT_VERTICES UNSET
#define CMN_MAX_MESH_OUTPUT_PRIMITIVES UNSET

#ifdef __cplusplus
#include <glm/glm.hpp>

using namespace glm;
#endif




#endif //LOD_TERRAIN_RENDERING_THESIS_PROJECT_COMMON_H
