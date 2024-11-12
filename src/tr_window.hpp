#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

class Window {
private:
    int m_Width, m_Height;
    std::string m_Name;

    GLFWwindow *m_Window;

    void Init();

public:
    Window(int width, int height, std::string name);

    GLFWwindow *getGLFWWindow();

    glm::vec2 getDimensions();

    ~Window();
};