#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 texCoord;
out vec3 fragPos;
out vec3 fragNormal;
out vec3 gouraudColor;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform vec3 viewPos;
uniform int shadingMode;

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

vec3 CalculateLighting(vec3 normal, vec3 fragPos);
vec3 CalculateStreetLight(vec3 normal, vec3 fragPos);
vec3 CalculateHeadlight(vec3 normal, vec3 fragPos, CarHeadlight headlight);

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    fragPos = vec3(model * vec4(aPos, 1.0));
    fragNormal = mat3(transpose(inverse(model))) * aNormal;
    texCoord = aTexCoord;

    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));

    TBN = mat3(T, B, N);

    if (shadingMode == 0) {
        gouraudColor = CalculateLighting(normalize(fragNormal), fragPos);
    } else {
        gouraudColor = vec3(0.0);
    }
}

vec3 CalculateLighting(vec3 normal, vec3 fragPos) {
    vec3 norm = normalize(normal);
    vec3 light = normalize(-lightDir);
    vec3 viewD = normalize(viewPos - fragPos);

    vec3 ambient = ambientColor;

    float diff = max(dot(norm, light), 0.0);
    vec3 diffuse = lightColor * diff;

    vec3 reflectD = reflect(-light, norm);
    float spec = pow(max(dot(viewD, reflectD), 0.0), 32.0);
    vec3 specular = lightColor * spec;

    vec3 streetLighting = CalculateStreetLight(norm, fragPos);
    vec3 headlightLighting = CalculateHeadlight(norm, fragPos, headlightLeft) +
                             CalculateHeadlight(norm, fragPos, headlightRight);

    return ambient + diffuse + specular + streetLighting + headlightLighting;
}

vec3 CalculateHeadlight(vec3 normal, vec3 fragPos, CarHeadlight headlight)
{
    vec3 toLight = headlight.position - fragPos;
    float distance = length(toLight);
    vec3 lightDirNorm = normalize(toLight);

    float theta = dot(lightDirNorm, normalize(-headlight.direction));

    float gaussianFactor = exp(-pow(distance / headlight.radius, 2.5));

    float spotlightIntensity = smoothstep(headlight.outerCutoff, headlight.cutoff, theta);
    spotlightIntensity = pow(spotlightIntensity, 2.0); // Wzmocnienie efektu

    float halo = exp(-pow(distance / (headlight.radius * 0.7), 2.0)) * 0.2;

    vec3 reflectDir = reflect(-lightDirNorm, normal);
    float spec = pow(max(dot(viewPos - fragPos, reflectDir), 0.0), 16.0);

    float fog = exp(-distance * 0.04);

    return headlight.color * spotlightIntensity * gaussianFactor * (1.0 + spec + halo) * fog;
}



vec3 CalculateStreetLight(vec3 normal, vec3 fragPos)
{
    vec3 toLight = streetLight.position - fragPos;
    float distance = length(toLight);
    vec3 lightDirNorm = normalize(toLight);

    float theta = dot(lightDirNorm, normalize(-streetLight.direction));

    float spotlightIntensity = smoothstep(streetLight.outerCutoff, streetLight.cutoff, theta);

    float attenuation = 1.0 / (1.0 + streetLight.radius * distance + (streetLight.radius * distance * distance));

    float diff = max(dot(normal, lightDirNorm), 0.0);

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDirNorm, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 ambient = streetLight.color * 0.2;
    vec3 diffuse = streetLight.color * diff * spotlightIntensity;
    vec3 specular = streetLight.color * spec * 0.5 * spotlightIntensity;

    return ambient*attenuation + diffuse* attenuation + specular* attenuation;
}
