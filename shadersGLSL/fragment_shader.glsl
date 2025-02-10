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

vec3 CalculatePhongLighting(vec3 normal, vec3 fragPos, vec3 objectColor, float roughness);
vec3 CalculateStreetLight(vec3 normal, vec3 fragPos);

void main()
{
    // Pobieranie kolorów z tekstur
    vec3 albedo = texture(textureAlbedo, texCoord).rgb;
    vec3 normalMap = texture(textureNormal, texCoord).rgb * 2.0 - 1.0; // Konwersja z [0,1] -> [-1,1]
    float roughness = texture(textureRoughness, texCoord).r;


    // Normalne: jeœli mamy mapê normalnych, ³¹czymy j¹ z geometri¹ modelu
    vec3 norm = normalize(normalMap);

    // Obliczenie œwiat³a Phonga
     vec3 lighting = CalculatePhongLighting(normalize(fragNormal), fragPos, albedo, roughness);

    // Dodanie œwiat³a latarni
    lighting += CalculateStreetLight(normalize(fragNormal), fragPos);

    // Ustawienie koñcowego koloru
    FragColor = vec4(lighting, 1.0);
}


vec3 CalculateStreetLight(vec3 normal, vec3 fragPos)
{
    // Kierunek od fragmentu do Ÿród³a œwiat³a
    vec3 toLight = streetLight.position - fragPos;
    float distance = length(toLight);
    
    // Normalizacja kierunku œwiat³a
    vec3 lightDirNorm = normalize(toLight);

    // K¹t miêdzy kierunkiem œwiat³a a jego sto¿kiem
    float theta = dot(lightDirNorm, normalize(-streetLight.direction));

    // Wygaszanie œwiat³a w zale¿noœci od odleg³oœci
    float attenuation = exp(-pow(distance / streetLight.radius, 2.0));

    // Jeœli fragment jest wewn¹trz sto¿ka œwiat³a
    if (theta > streetLight.cutoff)
    {
        // Wartoœæ spotlighta (przejœcie od krawêdzi do œrodka)
        float spotlightIntensity = (theta - streetLight.outerCutoff) / (streetLight.cutoff - streetLight.outerCutoff);
        spotlightIntensity = clamp(spotlightIntensity, 0.0, 1.0);

        // Nowe wygaszanie – silne œwiat³o w œrodku, malej¹ce ku brzegom
        float spotlightFade = smoothstep(0.0, 1.0, pow(spotlightIntensity, 3.0));

        // Dyfuzyjne oœwietlenie (bardziej miêkkie)
        float diff = max(dot(normal, lightDirNorm), 0.0);

        // Nowe t³umienie – im dalej od œrodka sto¿ka, tym mniej œwiat³a
        float centerIntensity = 1.0 - clamp(distance / (streetLight.radius * 0.8), 0.0, 1.0);
        centerIntensity = pow(centerIntensity, 2.0); // Wzmocnienie efektu

        // Finalne œwiat³o
        return streetLight.color * diff * attenuation * spotlightFade * centerIntensity;
    }

    return vec3(0.0); // Jeœli poza zakresem, brak œwiat³a
}



vec3 CalculatePhongLighting(vec3 normal, vec3 fragPos, vec3 objectColor, float roughness)
{
    vec3 norm = normalize(normal);
    vec3 lightDirNorm = normalize(-lightDir);

    // Oœwietlenie ambientowe
    vec3 ambient = ambientColor * objectColor * 0.5;

    // Oœwietlenie dyfuzyjne
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = lightColor * diff * objectColor;

    // Oœwietlenie specularne (odbicia œwiat³a) z roughness
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDirNorm, norm);
    float shininess = mix(4.0, 32.0, 1.0 - roughness); // Chropowatoœæ wp³ywa na rozproszenie œwiat³a
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = lightColor * spec * vec3(1.0);

    return ambient + diffuse + specular;
}
