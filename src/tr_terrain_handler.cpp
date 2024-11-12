#include "tr_terrain_handler.hpp"

#include <fstream>
#include <vector>
#include <glm/glm.hpp>

void TerrainHandler::LoadTerrain(std::string path, Terrain &dst) {
    std::ifstream inputfile(path, std::ios::binary);

    std::vector<char> bytes(
            (std::istreambuf_iterator<char>(inputfile)),
            (std::istreambuf_iterator<char>()));

    dst.data = new float[bytes.size() / 2];
    dst.size = bytes.size() / 2;

    int i = 0;
    while(i < bytes.size()) {
        uint8_t x = bytes[i],
                y = bytes[i + 1];

        float val = ((int16_t) ((((uint16_t) x << 8)) | (((uint16_t) y))));

        if (val < -10)
            val = -10;

        dst.data[i / 2] = val;

        i += 2;
    }

    inputfile.close();
}