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

vec3 CalculatePhongLighting(vec3 normal, vec3 fragPos, vec3 objectColor, float roughness);

void main()
{
    // Pobieranie kolorów z tekstur
    vec3 albedo = texture(textureAlbedo, texCoord).rgb;
    vec3 normalMap = texture(textureNormal, texCoord).rgb * 2.0 - 1.0; // Konwersja z [0,1] -> [-1,1]
    float roughness = texture(textureRoughness, texCoord).r;

    // Ustalenie koloru obiektu
    vec3 objectColor = isSphere ? vec3(0.5, 0.7, 1.0) : albedo;

    // Normalne: jeœli mamy mapê normalnych, ³¹czymy j¹ z geometri¹ modelu
    vec3 norm = normalize(isSphere ? fragNormal : normalMap);

    // Obliczenie œwiat³a Phonga
    vec3 lighting = CalculatePhongLighting(normalize(fragNormal), fragPos, objectColor,roughness);

    // Ustawienie koñcowego koloru
    FragColor = vec4(lighting, 1.0);
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
