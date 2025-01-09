#ifndef LOD_TERRAIN_RENDERING_THESIS_PROJECT_COMMON_H
#define LOD_TERRAIN_RENDERING_THESIS_PROJECT_COMMON_H

#define UNSET 1

#define MAX_PREFERRED_MESH_WORK_GROUP_INVOCATIONS 32
#define MAX_PREFERRED_TASK_WORK_GROUP_INVOCATIONS 32
#define MAX_MESH_OUTPUT_VERTICES 256
#define MAX_MESH_OUTPUT_PRIMITIVES 256

#define MESHLETS_PER_TASK_INVOCATION 2


// if encoded meshlets is not used, this will be removed




#ifdef __cplusplus
#include <glm/glm.hpp>

using namespace glm;
#endif

#define MESHLET_LENGTH 9

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

#define RENDEROPTIONS_DRAWING_MODE_HEIGHT 0
#define RENDEROPTIONS_DRAWING_MODE_MESHLET 1
#define RENDEROPTIONS_DRAWING_MODE_LOD 2

struct RenderOptions {
	int DrawingMode;
};

// offset can just be determined by k * MAX_MESHLET_LENGTH both in x and y


// Utils

const float PI = 3.1415926;


#ifdef __cplusplus
static
#endif
vec3 polarToCartesian(float lat, float lon, float height){
    float r = 50000 + height * 1 * 50000 / 6371000;
    return vec3( r * cos(lat) * sin(lon),
                 r * sin(lat),
                 r * cos(lat) * cos(lon)
                  );
}

#ifdef __cplusplus
static
vec2 localPolarOffset(uvec2 offset){
    offset.y = 1200 - offset.y;
    vec2 res = ((vec2)offset * PI ) * ( 1.0f / 180.0f * 1200.0f);
    return res;
}
#else
vec2 localPolarOffset(uvec2 offset){
    offset.y = 1200 - offset.y;
    vec2 res = (offset * PI ) / (180 * 1200);
    return res;
}

vec3 vertexToPolarToCartesian(uvec2 offset, float lat, float lon, float height){
    offset.y = 1200 - offset.y;
    vec2 res = (offset * PI ) / (180 * 1200) + vec2(lon, lat);

    float r = 50000 + height * 1 * 50000 / 6371000;
    return vec3( r * cos(res.y) * sin(res.x),
                 r * sin(res.y),
                 r * cos(res.y) * cos(res.x)
                  );
}
#endif


#ifdef __cplusplus
static
#endif
vec3 generalMeshletPosition(MeshletDescription meshlet, float latitude, float longitude, float height) {
    vec2 polarOffset = localPolarOffset(meshlet.Offset);
    return polarToCartesian(latitude + polarOffset.y, longitude + polarOffset.x, height);
}


#ifdef __cplusplus
static
#endif
float observerHorizonDistance(vec3 observerPosition){
    float height = length(observerPosition) - 50000;

    return sqrt(2 * 50000 * height + height*height);
}


#ifdef __cplusplus
static
#endif
bool shouldCull(MeshletDescription meshletDescription, float latitude, float longitude, float height, vec3 observerPosition) {
    vec3 meshletPos = generalMeshletPosition(meshletDescription, latitude, longitude, height);

    float dist = length(meshletPos - observerPosition);

    return dist > 1.5f * observerHorizonDistance(observerPosition);
}

#ifdef __cplusplus
static
#endif
bool shouldCull(MeshletDescription meshletDescription, float latitude, float longitude, float height, vec3 observerPosition, float observerHorizonDistanceCalculated) {
    vec3 meshletPos = generalMeshletPosition(meshletDescription, latitude, longitude, height);

    float dist = length(meshletPos - observerPosition);

    return dist > 1.5f * observerHorizonDistanceCalculated;
}


#endif //LOD_TERRAIN_RENDERING_THESIS_PROJECT_COMMON_H
