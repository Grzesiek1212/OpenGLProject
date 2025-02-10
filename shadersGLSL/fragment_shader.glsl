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
    // Pobieranie kolor�w z tekstur
    vec3 albedo = texture(textureAlbedo, texCoord).rgb;
    vec3 normalMap = texture(textureNormal, texCoord).rgb * 2.0 - 1.0; // Konwersja z [0,1] -> [-1,1]
    float roughness = texture(textureRoughness, texCoord).r;

    // Ustalenie koloru obiektu
    vec3 objectColor = isSphere ? vec3(0.5, 0.7, 1.0) : albedo;

    // Normalne: je�li mamy map� normalnych, ��czymy j� z geometri� modelu
    vec3 norm = normalize(isSphere ? fragNormal : normalMap);

    // Obliczenie �wiat�a Phonga
    vec3 lighting = CalculatePhongLighting(normalize(fragNormal), fragPos, objectColor,roughness);

    // Ustawienie ko�cowego koloru
    FragColor = vec4(lighting, 1.0);
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
