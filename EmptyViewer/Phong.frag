#version 330 core
in vec3 FragPos;
in vec3 Normal;

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float p;
uniform float Ia;
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform vec3 viewPos;

out vec4 FragColor;

void main() {
    // Ambient
    vec3 ambient = ka * Ia;

    // Diffuse
    vec3 n = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(n, lightDir), 0.0);
    vec3 diffuse = kd * diff * lightColor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 h = normalize(lightDir + viewDir);
    float spec = pow(max(dot(n, h), 0.0), p);
    vec3 specular = ks * spec * lightColor;

    vec3 color = ambient + diffuse + specular;

    // Gamma correction
    float gamma = 2.2;
    color = pow(color, vec3(1.0 / gamma));

    FragColor = vec4(color, 1.0);
}