#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 fragPos;
in vec3 fragNormal;

uniform bool isSphere;

uniform sampler2D textureAlbedo;
uniform sampler2D textureNormal;
uniform sampler2D textureRoughness;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform vec3 viewPos;

struct StreetLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float cutoff;
    float outerCutoff;
    float radius;
};
uniform StreetLight streetLight;

struct CarHeadlight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float cutoff;
    float outerCutoff;
    float radius;
};
uniform CarHeadlight headlightLeft;
uniform CarHeadlight headlightRight;


vec3 CalculatePhongLighting(vec3 normal, vec3 fragPos, vec3 objectColor, float roughness);
vec3 CalculateStreetLight(vec3 normal, vec3 fragPos);
vec3 CalculateHeadlight(vec3 normal, vec3 fragPos, CarHeadlight headlight);

void main()
{
    // Pobieranie kolor�w z tekstur
    vec3 albedo = texture(textureAlbedo, texCoord).rgb;
    vec3 normalMap = texture(textureNormal, texCoord).rgb * 2.0 - 1.0; // Konwersja z [0,1] -> [-1,1]
    float roughness = texture(textureRoughness, texCoord).r;


    // Normalne: je�li mamy map� normalnych, ��czymy j� z geometri� modelu
    vec3 norm = normalize(normalMap);

    // Obliczenie �wiat�a Phonga
     vec3 lighting = CalculatePhongLighting(normalize(fragNormal), fragPos, albedo, roughness);

    // Dodanie �wiat�a latarni
    lighting += CalculateStreetLight(normalize(fragNormal), fragPos);

     // Dodanie �wiate� reflektor�w
    lighting += CalculateHeadlight(normalize(fragNormal), fragPos, headlightLeft);
    lighting += CalculateHeadlight(normalize(fragNormal), fragPos, headlightRight);

    // Ustawienie ko�cowego koloru
    FragColor = vec4(lighting, 1.0);
}


vec3 CalculateHeadlight(vec3 normal, vec3 fragPos, CarHeadlight headlight)
{
    vec3 toLight = headlight.position - fragPos;
    float distance = length(toLight);
    vec3 lightDirNorm = normalize(toLight);

    float theta = dot(lightDirNorm, normalize(-headlight.direction));

    // Gaussowskie t�umienie �wiat�a (realistyczne rozproszenie)
    float gaussianFactor = exp(-pow(distance / headlight.radius, 2.5));

    // Intensywno�� sto�ka �wiat�a (p�ynne przej�cie)
    float spotlightIntensity = smoothstep(headlight.outerCutoff, headlight.cutoff, theta);
    spotlightIntensity = pow(spotlightIntensity, 2.0); // Wzmocnienie efektu

    // Efekt halo - dodajemy lekkie po�wiaty wok� �wiate�
    float halo = exp(-pow(distance / (headlight.radius * 0.7), 2.0)) * 0.2;

    // �wiat�o odbite od pod�o�a
    vec3 reflectDir = reflect(-lightDirNorm, normal);
    float spec = pow(max(dot(viewPos - fragPos, reflectDir), 0.0), 16.0);

    // Efekt mg�y - zmniejsza intensywno�� �wiat�a wraz z odleg�o�ci�
    float fog = exp(-distance * 0.04);

    // Finalne �wiat�o reflektor�w
    return headlight.color * spotlightIntensity * gaussianFactor * (1.0 + spec + halo) * fog;
}



vec3 CalculateStreetLight(vec3 normal, vec3 fragPos)
{
    // Kierunek od fragmentu do �r�d�a �wiat�a
    vec3 toLight = streetLight.position - fragPos;
    float distance = length(toLight);
    
    // Normalizacja kierunku �wiat�a
    vec3 lightDirNorm = normalize(toLight);

    // K�t mi�dzy kierunkiem �wiat�a a jego sto�kiem
    float theta = dot(lightDirNorm, normalize(-streetLight.direction));

    // Wygaszanie �wiat�a w zale�no�ci od odleg�o�ci
    float attenuation = exp(-pow(distance / streetLight.radius, 2.0));

    // Je�li fragment jest wewn�trz sto�ka �wiat�a
    if (theta > streetLight.cutoff)
    {
        // Warto�� spotlighta (przej�cie od kraw�dzi do �rodka)
        float spotlightIntensity = (theta - streetLight.outerCutoff) / (streetLight.cutoff - streetLight.outerCutoff);
        spotlightIntensity = clamp(spotlightIntensity, 0.0, 1.0);

        // Nowe wygaszanie � silne �wiat�o w �rodku, malej�ce ku brzegom
        float spotlightFade = smoothstep(0.0, 1.0, pow(spotlightIntensity, 3.0));

        // Dyfuzyjne o�wietlenie (bardziej mi�kkie)
        float diff = max(dot(normal, lightDirNorm), 0.0);

        // Nowe t�umienie � im dalej od �rodka sto�ka, tym mniej �wiat�a
        float centerIntensity = 1.0 - clamp(distance / (streetLight.radius * 0.8), 0.0, 1.0);
        centerIntensity = pow(centerIntensity, 2.0); // Wzmocnienie efektu

        // Finalne �wiat�o
        return streetLight.color * diff * attenuation * spotlightFade * centerIntensity;
    }

    return vec3(0.0); // Je�li poza zakresem, brak �wiat�a
}



vec3 CalculatePhongLighting(vec3 normal, vec3 fragPos, vec3 objectColor, float roughness)
{
    vec3 norm = normalize(normal);
    vec3 lightDirNorm = normalize(-lightDir);

    // O�wietlenie ambientowe
    vec3 ambient = ambientColor * objectColor * 0.5;

    // O�wietlenie dyfuzyjne
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = lightColor * diff * objectColor;

    // O�wietlenie specularne (odbicia �wiat�a) z roughness
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDirNorm, norm);
    float shininess = mix(4.0, 32.0, 1.0 - roughness); // Chropowato�� wp�ywa na rozproszenie �wiat�a
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = lightColor * spec * vec3(1.0);

    return ambient + diffuse + specular;
}
