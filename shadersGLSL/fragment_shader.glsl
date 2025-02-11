#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 fragPos;
in vec3 fragNormal;
in vec3 gouraudColor;
uniform int shadingMode;


uniform float fogDensity;
uniform vec3 fogColor;
uniform vec3 viewPos;

uniform sampler2D textureAlbedo;
uniform sampler2D textureNormal;
uniform sampler2D textureRoughness;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;

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
    // Pobieranie kolor w z tekstur
    vec3 albedo = texture(textureAlbedo, texCoord).rgb;
    vec3 normalMap = texture(textureNormal, texCoord).rgb * 2.0 - 1.0; // Konwersja z [0,1] -> [-1,1]
    float roughness = texture(textureRoughness, texCoord).r;

    vec3 lighting;

    if (shadingMode == 0) {
        lighting = gouraudColor * albedo;
    } else {
       lighting = CalculatePhongLighting(normalize(fragNormal), fragPos, albedo, roughness);
    
       // Dodanie światła latarni i reflektorów
       lighting += CalculateStreetLight(normalize(fragNormal), fragPos);
       lighting += CalculateHeadlight(normalize(fragNormal), fragPos, headlightLeft);
       lighting += CalculateHeadlight(normalize(fragNormal), fragPos, headlightRight);
	}

    // Obliczenie odleg o ci fragmentu od kamery
    float distance = length(viewPos - fragPos);

    // Ulepszona funkcja t umienia mg y (lepszy efekt)
    float fogFactor = exp(-pow(distance * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Miksowanie  wiat a sceny z mg  
    vec3 finalColor = mix(fogColor, lighting, fogFactor);

    // Ustawienie ko cowego koloru
    FragColor = vec4(finalColor, 1.0);
}


vec3 CalculateHeadlight(vec3 normal, vec3 fragPos, CarHeadlight headlight)
{
    vec3 toLight = headlight.position - fragPos;
    float distance = length(toLight);
    vec3 lightDirNorm = normalize(toLight);

    float theta = dot(lightDirNorm, normalize(-headlight.direction));

    // Gaussowskie t umienie  wiat a (realistyczne rozproszenie)
    float gaussianFactor = exp(-pow(distance / headlight.radius, 2.5));

    // Intensywno   sto ka  wiat a (p ynne przej cie)
    float spotlightIntensity = smoothstep(headlight.outerCutoff, headlight.cutoff, theta);
    spotlightIntensity = pow(spotlightIntensity, 2.0); // Wzmocnienie efektu

    // Efekt halo - dodajemy lekkie po wiaty wok   wiate 
    float halo = exp(-pow(distance / (headlight.radius * 0.7), 2.0)) * 0.2;

    //  wiat o odbite od pod o a
    vec3 reflectDir = reflect(-lightDirNorm, normal);
    float spec = pow(max(dot(viewPos - fragPos, reflectDir), 0.0), 16.0);

    // Efekt mg y - zmniejsza intensywno    wiat a wraz z odleg o ci 
    float fog = exp(-distance * 0.04);

    // Finalne  wiat o reflektor w
    return headlight.color * spotlightIntensity * gaussianFactor * (1.0 + spec + halo) * fog;
}



vec3 CalculateStreetLight(vec3 normal, vec3 fragPos)
{
    // Kierunek od fragmentu do  r d a  wiat a
    vec3 toLight = streetLight.position - fragPos;
    float distance = length(toLight);
    vec3 lightDirNorm = normalize(toLight);

    // K t mi dzy kierunkiem latarni a fragmentem sceny
    float theta = dot(lightDirNorm, normalize(-streetLight.direction));

    // Sprawdzenie, czy fragment jest w sto ku  wiat a latarni
    float spotlightIntensity = smoothstep(streetLight.outerCutoff, streetLight.cutoff, theta);

    // Wygaszanie  wiat a na podstawie odleg o ci
    float attenuation = 1.0 / (1.0 + streetLight.radius * distance + (streetLight.radius * distance * distance));

    // O wietlenie dyfuzyjne (rozproszone)
    float diff = max(dot(normal, lightDirNorm), 0.0);

    // O wietlenie specularne (odbicia  wiat a)
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDirNorm, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    // Finalne  wiat o latarni =  wiat o dyfuzyjne + specularne + zanikanie
    vec3 ambient = streetLight.color * 0.2;
    vec3 diffuse = streetLight.color * diff * spotlightIntensity;
    vec3 specular = streetLight.color * spec * 0.5 * spotlightIntensity;

    return ambient*attenuation + diffuse* attenuation + specular* attenuation;
}




vec3 CalculatePhongLighting(vec3 normal, vec3 fragPos, vec3 objectColor, float roughness)
{
    vec3 norm = normalize(normal);
    vec3 lightDirNorm = normalize(-lightDir);

    // O wietlenie ambientowe
    vec3 ambient = ambientColor * objectColor * 0.5;

    // O wietlenie dyfuzyjne
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = lightColor * diff * objectColor;

    // O wietlenie specularne (odbicia  wiat a) z roughness
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDirNorm, norm);
    float shininess = mix(4.0, 32.0, 1.0 - roughness); // Chropowato   wp ywa na rozproszenie  wiat a
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = lightColor * spec * vec3(1.0);

    return ambient + diffuse + specular;
}
