#pragma once

#include <string>

struct Terrain{
public:
    float size;
    float *data;
};

class TerrainHandler {
private:

public:
    static void LoadTerrain(std::string path, Terrain &dst);
};
