#version 330 core

in vec3 vFragPos;
in vec3 vNormal;

// Directional light (direction FROM world TO light)
uniform vec3  uLightDir;
uniform vec3  uLightColor;
uniform float uAmbientStrength;

// Camera
uniform vec3 uViewPos;

// Material
uniform vec3  uMatDiffuse;
uniform vec3  uMatSpecular;
uniform float uMatShininess;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);           // direction to light
    vec3 V = normalize(uViewPos - vFragPos); // direction to viewer
    vec3 H = normalize(L + V);               // Blinn-Phong half-vector

    // Ambient
    vec3 ambient = uAmbientStrength * uLightColor * uMatDiffuse;

    // Diffuse
    float diff   = max(dot(N, L), 0.0);
    vec3 diffuse = diff * uLightColor * uMatDiffuse;

    // Specular (Blinn-Phong)
    float spec   = pow(max(dot(N, H), 0.0), uMatShininess);
    vec3 specular = spec * uLightColor * uMatSpecular;

    vec3 result = ambient + diffuse + specular;
    FragColor   = vec4(result, 1.0);
}
