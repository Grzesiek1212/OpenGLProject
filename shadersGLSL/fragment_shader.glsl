#version 330

out vec4 FragColor;

in vec2 texCoord;
in vec3 FragPos;
in vec3 Normal;
uniform bool isSphere;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

uniform sampler2D textureAlbedo;
uniform sampler2D textureNormal;
uniform sampler2D textureRoughness;

void main()
{
    vec3 color = texture(textureAlbedo, texCoord).rgb;
    vec3 normalMap = texture(textureNormal, texCoord).rgb * 2.0 - 1.0;
    float roughness = texture(textureRoughness, texCoord).r;

    if (isSphere)
        FragColor = vec4(0.5f, 0.7f, 1.0f, 1.0f);
    else
        FragColor = texture(texture_diffuse1, texCoord);
}