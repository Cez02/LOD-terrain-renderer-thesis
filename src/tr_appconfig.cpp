#include "tr_appconfig.hpp"

#include <iostream>

AppConfig APP_CONFIG;

void AppConfig::print() {

    std::cout << "App configuration:" << std::endl;

    std::cout << "Max task invocations: " << m_MeshShaderConfig.m_MaxTaskWorkGroupInvocations << std::endl;
}