// SEA

#version 330 core

in vec4 ourColor;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

struct Light
{
    int lightType;
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material
{
    sampler2D diffuseT;
    sampler2D specularT;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material water;
uniform Light sun;
uniform vec3 viewPos;

void main()
{
    // ambient
    vec3 ambient = sun.ambient * water.diffuse;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(sun.position - FragPos);
    //vec3 lightDir = normalize(sun.direction);               // uncomment for directional light. Comment for
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = sun.diffuse * (diff * water.diffuse);

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), water.shininess);
    vec3 specular = sun.specular * (spec * water.specular);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 0.8);
}
