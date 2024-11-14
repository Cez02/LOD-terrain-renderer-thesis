//
// Created by cezary on 13.11.24.
//

#include "tr_scenedata.hpp"

std::vector<Heightmap> &SceneData::getHeightmaps() {
    return m_Heightmaps;
}

Camera &SceneData::getCamera() {
    return m_Camera;
}

void SceneData::Deinit() {
    for(auto &heightmap : m_Heightmaps){
        heightmap.Deinit();
    }
}
