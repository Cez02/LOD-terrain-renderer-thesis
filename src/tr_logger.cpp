#include "tr_logger.hpp"

#include <string>
#include <iostream>

void log(std::string message){
    std::cout << "\033[0m" << message << std::endl;
}

void logWarning(std::string message){
    std::cout << "\033[1;33m" << message << "\033[0m" << std::endl;
}

void logError(std::string message){
    std::cout << "\033[1;31m" << message << "\033[0m" << std::endl;
}
