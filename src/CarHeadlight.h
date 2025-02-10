#ifndef CAR_HEADLIGHT_H
#define CAR_HEADLIGHT_H

#include <glm/glm.hpp>

class CarHeadlight {
public:
    glm::vec3 position; 
    glm::vec3 direction;
    glm::vec3 color;    
    float intensity;    // Jasno�� reflektora
    float cutoff;       // K�t sto�ka �wiat�a (wewn�trzny)
    float outerCutoff;  // Zewn�trzny k�t (�agodne wygasanie)
    float radius;       // Zasi�g �wiat�a

    CarHeadlight(glm::vec3 pos, glm::vec3 dir, glm::vec3 col, float intens, float cut, float outerCut, float rad)
        : position(pos), direction(glm::normalize(dir)), color(col), intensity(intens), cutoff(cut), outerCutoff(outerCut), radius(rad) {}
};

#endif
