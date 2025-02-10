#ifndef STREETLAMP_H
#define STREETLAMP_H

#include <glm/glm.hpp>

class StreetLamp {
public:
    glm::vec3 position; // Pozycja latarni
    glm::vec3 direction; // Kierunek �wiat�a (musi by� w d�)
    glm::vec3 color;    // Kolor �wiat�a
    float intensity;    // Intensywno�� �wiat�a
    float cutoff;       // K�t sto�ka �wiat�a
    float outerCutoff;  // Zewn�trzny k�t sto�ka (mi�kkie przej�cie)
    float radius;       // NOWE: Promie� wp�ywu �wiat�a

    StreetLamp(glm::vec3 pos, glm::vec3 dir, glm::vec3 col, float intens, float cut, float outerCut, float rad)
        : position(pos), direction(glm::normalize(dir)), color(col), intensity(intens),
        cutoff(cut), outerCutoff(outerCut), radius(rad) {}
};

#endif
