//
// Created by JeremyGuo on 2022/2/22.
//

#include "../lib/glfwApp/glfwApp.h"

class MyApp : public glfw::glfwApp {
};

int main(int argc, char **argv) {
    MyApp myApp;
    try {
        myApp.initialize();
        myApp.run();
    } catch (std::exception& e) {
        printException(e);
    }

    return 0;
}