#include <GLFW/glfw3.h>
#include "tr_window.hpp"

void Window::Init()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_Window = glfwCreateWindow(m_Width, m_Height, m_Name.c_str(), nullptr, nullptr);
}



Window::Window(int width, int height, std::string name)
    : m_Width(width), m_Height(height), m_Name(name)
{
    Init();
}

GLFWwindow *Window::getGLFWWindow()
{
    return m_Window;
}

Window::~Window()
{
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

glm::vec2 Window::getDimensions() {
    return glm::vec2(m_Width, m_Height);
}
