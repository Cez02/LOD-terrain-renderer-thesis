#pragma once

#include "tr_heightmap.hpp"
#include "tr_camera.hpp"

#include <vector>


class SceneData {
private:
    std::vector<Heightmap> m_Heightmaps;
    Camera m_Camera;

public:
    std::vector<Heightmap> &getHeightmaps();
    Camera &getCamera();

    void Deinit();
};
