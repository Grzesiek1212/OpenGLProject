#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

class Renderer {
public:
    static GLFWwindow* Initialize();
};

// Deklaracja funkcji obs³ugi myszy
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

#endif
