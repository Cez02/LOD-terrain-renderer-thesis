//
// Created by cezary-czubala on 12/27/24.
//

#include "tr_guihandler.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include "tr_logger.hpp"

#include <iomanip>

#include "imgui.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <sstream>

#include "tr_camera.hpp"


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

static void ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    ImVec2 pos[4] =
    {
        center + ImRotate(ImVec2(-size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
        center + ImRotate(ImVec2(+size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
        center + ImRotate(ImVec2(+size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a),
        center + ImRotate(ImVec2(-size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a)
    };
    ImVec2 uvs[4] =
    {
        ImVec2(0.0f, 0.0f),
        ImVec2(1.0f, 0.0f),
        ImVec2(1.0f, 1.0f),
        ImVec2(0.0f, 1.0f)
    };

    draw_list->AddImageQuad(tex_id, pos[0], pos[1], pos[2], pos[3], uvs[0], uvs[1], uvs[2], uvs[3], IM_COL32_WHITE);
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

    stream.clear();
    stream = std::stringstream();
    stream << std::fixed << m_GUIData.m_MeshShaderInvocations;
    ImGui::Text(("  Mesh shader invocs: " + quickAddThousandSeparators(stream.str(), '\'')).c_str());

    stream.clear();
    stream = std::stringstream();
    stream << std::fixed << m_GUIData.m_TaskShaderInvocations;
    ImGui::Text(("  Task shader invocs: " + quickAddThousandSeparators(stream.str(), '\'')).c_str());

    // player coords

    float longitude = m_GUIData.m_PlayerPolarPosition.x * 180 / glm::pi<float>();
    float latitude = m_GUIData.m_PlayerPolarPosition.y * 90 / glm::half_pi<float>();
    // latitude = latitude*2 - 90;

    if (longitude > 0)
    {
        stream.clear();
        stream = std::stringstream();
        stream << std::fixed << "E " << ((int)longitude) << "° " <<  (((int)( 100 * longitude)) % 100) << "'";
        ImGui::Text(("Longitude: " + stream.str()).c_str());
    }
    else
    {
        longitude *= -1;
        stream.clear();
        stream = std::stringstream();
        stream << std::fixed << "W " << ((int)longitude) << "° " <<  (((int)( 100 * longitude)) % 100) << "'";
        ImGui::Text(("Longitude: " + stream.str()).c_str());
    }

    if (latitude > 0)
    {
        stream.clear();
        stream = std::stringstream();
        stream << std::fixed << "N " << ((int)latitude) << "° " <<  (((int)( 100 * latitude)) % 100) << "'";
        ImGui::Text((" Latitude: " + stream.str()).c_str());
    }
    else
    {
        latitude *= -1;
        stream.clear();
        stream = std::stringstream();
        stream << std::fixed << "S " << ((int)latitude) << "° " <<  (((int)( 100 * latitude)) % 100) << "'";
        ImGui::Text((" Latitude: " + stream.str()).c_str());
    }

    float aboveSea = glm::length(Camera::instance->m_Position) - 50000;

    aboveSea *= 1000;

    if (aboveSea >= 0)
    {
        stream.clear();
        stream = std::stringstream();
        stream << std::fixed << aboveSea << "m";
        ImGui::Text((" Above sea level: " + stream.str()).c_str());
    }
    else
    {
        stream.clear();
        stream = std::stringstream();
        stream << std::fixed << -aboveSea << "m";
        ImGui::Text((" Below sea level: " + stream.str()).c_str());
    }

    //ImGui::Image((ImTextureID)(m_GUIData.m_CompassTexture->DS), ImVec2(m_GUIData.m_CompassTexture->Width, m_GUIData.m_CompassTexture->Height));

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos);
    const ImVec2 center = window->DC.CursorPos + ((bb.Max - bb.Min) * 0.5) + ImVec2(window->Size.x, 0) * 0.5f + ImVec2(0, m_GUIData.m_CompassTexture->Height * 0.6f);

    ImageRotated((ImTextureID)(m_GUIData.m_CompassTexture->DS), center, ImVec2(m_GUIData.m_CompassTexture->Width, m_GUIData.m_CompassTexture->Height), m_GUIData.m_CompassRotation );

    ImGui::End();


}
