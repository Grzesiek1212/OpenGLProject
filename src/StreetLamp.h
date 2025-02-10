#ifndef STREETLAMP_H
#define STREETLAMP_H

#include <glm/glm.hpp>

class StreetLamp {
public:
    glm::vec3 position; // Pozycja latarni
    glm::vec3 direction; // Kierunek œwiat³a (musi byæ w dó³)
    glm::vec3 color;    // Kolor œwiat³a
    float intensity;    // Intensywnoœæ œwiat³a
    float cutoff;       // K¹t sto¿ka œwiat³a
    float outerCutoff;  // Zewnêtrzny k¹t sto¿ka (miêkkie przejœcie)
    float radius;       // NOWE: Promieñ wp³ywu œwiat³a

    StreetLamp(glm::vec3 pos, glm::vec3 dir, glm::vec3 col, float intens, float cut, float outerCut, float rad)
        : position(pos), direction(glm::normalize(dir)), color(col), intensity(intens),
        cutoff(cut), outerCutoff(outerCut), radius(rad) {}
};

#endif
