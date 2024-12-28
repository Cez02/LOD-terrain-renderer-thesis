//
// Created by cezary-czubala on 12/27/24.
//

#include "tr_guihandler.h"

#include <iomanip>

#include "imgui.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <sstream>


/*
*    float m_DeltaTime;
    uint m_RenderedTriangleCount;
    uint m_MeshletCount;
    bool m_ToggleMeshletColoring;

 *
 */

static std::string quickAddThousandSeparators(std::string value, char thousandSep = ',')
{
    int len = value.length();
    int dlen = 3;

    while (len > dlen) {
        value.insert(len - dlen, 1, thousandSep);
        dlen += 4;
        len += 1;
    }
    return value;
}

void GUIHandler::DrawGUI()
{
    ImGui::Begin("Terrain renderer");                          // Create a window called "Hello, world!" and append into it.

    // TODO: allow heightmap selection with zenity

    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << (m_GUIData.m_DeltaTime * 1000.0f) << " ms";
    ImGui::Text(("          Delta time: " + stream.str()).c_str());

    stream.clear();
    stream = std::stringstream();
    stream << std::fixed << std::setprecision(0) << (1.0f / m_GUIData.m_DeltaTime) << " FPS";
    ImGui::Text(("                 FPS: " + stream.str()).c_str());

    stream.clear();
    stream = std::stringstream();
    stream << std::fixed << m_GUIData.m_RenderedTriangleCount;
    ImGui::Text(("Generated primitives: " + quickAddThousandSeparators(stream.str(), '\'')).c_str());


    ImGui::End();


}
