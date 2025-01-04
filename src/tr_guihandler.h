//
// Created by cezary-czubala on 12/27/24.
//

#ifndef TR_GUIHANDLER_H
#define TR_GUIHANDLER_H

#include "tr_texture.h"
#include "glm/glm.hpp"

class GUI_Data
{
public:
    float m_DeltaTime;
    uint64_t m_RenderedTriangleCount = 0;
    uint m_MeshletCount;
    bool m_ToggleMeshletColoring;

    Texture *m_CompassTexture;
    float m_CompassRotation = 0;

    glm::vec2 m_PlayerPolarPosition;
};

class GUIHandler {
public:

    GUI_Data m_GUIData;
    void DrawGUI();
};



#endif //TR_GUIHANDLER_H
