#ifndef SPHERE_H
#define SPHERE_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shaders/Shader.h"

class Sphere {
public:
    Sphere(float radius, unsigned int sectors = 36, unsigned int stacks = 18);
    void Draw(Shader& shader);

private:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    void GenerateSphere(unsigned int sectors, unsigned int stacks);
    void setupSphere();
};

#endif
