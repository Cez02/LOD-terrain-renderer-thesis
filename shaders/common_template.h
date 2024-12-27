#ifndef LOD_TERRAIN_RENDERING_THESIS_PROJECT_COMMON_H
#define LOD_TERRAIN_RENDERING_THESIS_PROJECT_COMMON_H

#define UNSET 1

#define MAX_PREFERRED_MESH_WORK_GROUP_INVOCATIONS $m_MaxPreferredMeshWorkGroupInvocations
#define MAX_PREFERRED_TASK_WORK_GROUP_INVOCATIONS $m_MaxPreferredTaskWorkGroupInvocations
#define MAX_MESH_OUTPUT_VERTICES $m_MaxMeshOutputVertices
#define MAX_MESH_OUTPUT_PRIMITIVES $m_MaxMeshOutputPrimitives

// if encoded meshlets is not used, this will be removed
#define ENCODED_MESHLETS


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

// Encoded meshlet
// Offset - x, y, - uint8, uint8. It defines the offset in terms of meshlet units
// Dimensions - uint8, uint8. We don't need much more
// HeightmapDataOffset - uint. We need this for better data access

#define EncodedMeshletDescription uvec2
#define EncodedOffsetAndDimensions x
#define EncodedHeightmapDataOffset y

#ifndef __cplusplus
void decodeMeshlet( EncodedMeshletDescription encodedMeshletDescription,
                    inout MeshletDescription meshletDescription )
{
	meshletDescription.Offset.x = encodedMeshletDescription.EncodedOffsetAndDimensions & 255;
    meshletDescription.Offset.y = (encodedMeshletDescription.EncodedOffsetAndDimensions >> 8) & 255;

    meshletDescription.Offset *= (MESHLET_LENGTH - 1);

    meshletDescription.Dimensions.x = (encodedMeshletDescription.EncodedOffsetAndDimensions >> 16) & 255;
    meshletDescription.Dimensions.y = (encodedMeshletDescription.EncodedOffsetAndDimensions >> 24) & 255;

    meshletDescription.HeightmapDataOffset = encodedMeshletDescription.EncodedHeightmapDataOffset;
}

#else

inline EncodedMeshletDescription encodeMeshlet( MeshletDescription meshletDescription, uint meshletLength )
{
	EncodedMeshletDescription encodedMeshlet;

    encodedMeshlet.EncodedOffsetAndDimensions =
    	((meshletDescription.Offset.x / (meshletLength - 1 ))) |
        ((meshletDescription.Offset.y / (meshletLength - 1 )) << 8) |
        ((meshletDescription.Dimensions.x ) << 16) |
        ((meshletDescription.Dimensions.y ) << 24);

    encodedMeshlet.EncodedHeightmapDataOffset = meshletDescription.HeightmapDataOffset;

    return encodedMeshlet;
}
#endif

// offset can just be determined by k * MAX_MESHLET_LENGTH both in x and y


#endif //LOD_TERRAIN_RENDERING_THESIS_PROJECT_COMMON_H
