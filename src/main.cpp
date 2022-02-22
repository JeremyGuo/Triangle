//
// Created by JeremyGuo on 2022/2/22.
//

#include "../lib/glfwApp/glfwApp.h"

class MyApp : public glfw::glfwApp {
public:
    void initialize();

protected:
    void onDraw() override;
    void onUpdate() override;

    void initGraphicsPipeline();
//    void initFramebuffers();
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

void MyApp::initialize() {
    glfw::glfwApp::initialize();
    try {
        this->initGraphicsPipeline();
    } catch(...) {
        std::throw_with_nested("failed to init myApp");
    }
}

void MyApp::onDraw() {
}

void MyApp::onUpdate() {
}

void MyApp::initGraphicsPipeline() {

}
