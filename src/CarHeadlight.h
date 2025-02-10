#ifndef CAR_HEADLIGHT_H
#define CAR_HEADLIGHT_H

#include <glm/glm.hpp>

class CarHeadlight {
public:
    glm::vec3 position; 
    glm::vec3 direction;
    glm::vec3 color;    
    float intensity;    // Jasnoœæ reflektora
    float cutoff;       // K¹t sto¿ka œwiat³a (wewnêtrzny)
    float outerCutoff;  // Zewnêtrzny k¹t (³agodne wygasanie)
    float radius;       // Zasiêg œwiat³a

    CarHeadlight(glm::vec3 pos, glm::vec3 dir, glm::vec3 col, float intens, float cut, float outerCut, float rad)
        : position(pos), direction(glm::normalize(dir)), color(col), intensity(intens), cutoff(cut), outerCutoff(outerCut), radius(rad) {}
};

#endif
