#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 fragPos;
in vec3 fragNormal;
in vec3 gouraudColor;
in mat3 TBN;

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
uniform bool useBumpMapping;

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
    vec3 albedo = texture(textureAlbedo, texCoord).rgb;
    vec3 normalMap = texture(textureNormal, texCoord).rgb * 2.0 - 1.0;
    vec3 normal;
    if (useBumpMapping) {
        normal = normalize(TBN * normalMap);
    } else {
        normal = normalize(fragNormal);
    }

    float roughness = texture(textureRoughness, texCoord).r;

    vec3 lighting;

    if (shadingMode == 0) {
        lighting = gouraudColor * albedo;
    } else {
       lighting = CalculatePhongLighting(normal, fragPos, albedo, roughness);
       lighting += CalculateStreetLight(normal, fragPos);
       lighting += CalculateHeadlight(normal, fragPos, headlightLeft);
       lighting += CalculateHeadlight(normal, fragPos, headlightRight);
	}

    float distance = length(viewPos - fragPos);

    float fogFactor = exp(-pow(distance * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 finalColor = mix(fogColor, lighting, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}


vec3 CalculateHeadlight(vec3 normal, vec3 fragPos, CarHeadlight headlight)
{
    vec3 toLight = headlight.position - fragPos;
    float distance = length(toLight);
    vec3 lightDirNorm = normalize(toLight);

    float theta = dot(lightDirNorm, normalize(-headlight.direction));

    float gaussianFactor = exp(-pow(distance / headlight.radius, 2.5));

    float spotlightIntensity = smoothstep(headlight.outerCutoff, headlight.cutoff, theta);
    spotlightIntensity = pow(spotlightIntensity, 2.0);

    float halo = exp(-pow(distance / (headlight.radius * 0.7), 2.0)) * 0.2;

    vec3 reflectDir = reflect(-lightDirNorm, normal);
    float spec = pow(max(dot(normalize(viewPos - fragPos), reflectDir), 0.0), 16.0);

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




vec3 CalculatePhongLighting(vec3 normal, vec3 fragPos, vec3 objectColor, float roughness)
{
    vec3 norm = normal;
    vec3 lightDirNorm = normalize(-lightDir);

    vec3 ambient = ambientColor * objectColor * 0.7;

    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = lightColor * diff * objectColor;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDirNorm, norm);
    float shininess = mix(4.0, 32.0, 1.0 - roughness);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = lightColor * spec * vec3(1.0);

    return ambient + diffuse + specular;
}
