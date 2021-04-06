// TERRAIN

#version 330 core

out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

struct Light
{
    int lightType;      //   0: directional   1: point   2: spot
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
};

struct Material
{
    sampler2D diffuseT;
    vec3 diffuse;
    sampler2D specularT;
    vec3 specular;
    float shininess;
};

uniform Light sun;
uniform Material grass;
uniform Material rock;
uniform vec3 camPos;

vec4 getFragColor( Light light, vec3 diffuseMap, vec3 specularMap, float shininess, float alpha );

void main()
{
    float threshold = 0.6;      // grass-rock slope threshold
    float range     = 0.05;     // threshold mixing range
    float slope     = dot( normalize(Normal), normalize(vec3(Normal.x, Normal.y, 0.0)) );

    float rtf = 30;             // rock texture factor
    float gtf = 20;             // grass texture factor

    if(slope > threshold + range)   // ROCK
    {
        FragColor = getFragColor( sun, vec3(texture(rock.diffuseT, TexCoord/rtf)), vec3(texture(rock.specularT, TexCoord/rtf)), rock.shininess, 1.0 );
        //FragColor = getFragColor( sun, vec3(texture(rock.diffuseT, TexCoord/rtf)), rock.specular, rock.shininess, 1.0 );
    }
    else if (slope < threshold - range)   // GRASS
    {
        FragColor = getFragColor( sun, vec3(texture(grass.diffuseT, TexCoord/gtf)), vec3(texture(grass.specularT, TexCoord/gtf)), grass.shininess, 1.0 );
    }
    else   // MIXTURE
    {
        vec4 rockFrag  = getFragColor( sun, vec3(texture(rock.diffuseT, TexCoord/rtf)), vec3(texture(rock.specularT, TexCoord/rtf)), rock.shininess, 1.0 );
        //vec4 rockFrag  = getFragColor( sun, vec3(texture(rock.diffuseT, TexCoord/rtf)), rock.specular, rock.shininess, 1.0 );

        vec4 grassFrag = getFragColor( sun, vec3(texture(grass.diffuseT, TexCoord/gtf)), vec3(texture(grass.specularT, TexCoord/gtf)), grass.shininess, 1.0 );

        float ratio = (slope - threshold + range) / (2 * range);
        vec3 mix    = ratio * rockFrag.xyz + (1-ratio) * grassFrag.xyz;

        FragColor = vec4(mix, 1.0);
    }

}


vec4 DirectionalLightColor( Light light, vec3 diffuseMap, vec3 specularMap, float shininess, float alpha );
vec4 PointLightColor      ( Light light, vec3 diffuseMap, vec3 specularMap, float shininess, float alpha );
vec4 SpotLightColor       ( Light light, vec3 diffuseMap, vec3 specularMap, float shininess, float alpha );

vec4 getFragColor( Light light, vec3 diffuseMap, vec3 specularMap, float shininess, float alpha )
{
    if(light.lightType == 0) return DirectionalLightColor( light, diffuseMap, specularMap, shininess, alpha );
    if(light.lightType == 1) return PointLightColor      ( light, diffuseMap, specularMap, shininess, alpha );
    if(light.lightType == 2) return SpotLightColor       ( light, diffuseMap, specularMap, shininess, alpha );
}

vec4 DirectionalLightColor( Light light, vec3 diffuseMap, vec3 specularMap, float shininess, float alpha )
{
    // ----- Ambient lighting -----
    vec3 ambient = light.ambient * diffuseMap;

    // ----- Diffuse lighting -----
    vec3 lightDir = normalize(light.direction);
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseMap;

    // ----- Specular lighting -----
    vec3 viewDir = normalize(camPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = light.specular * spec * specularMap;

    // ----- Result -----
    return vec4(vec3(ambient + diffuse + specular), alpha);
}

vec4 PointLightColor( Light light, vec3 diffuseMap, vec3 specularMap, float shininess, float alpha )
{
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    // ----- Ambient lighting -----
    vec3 ambient = light.ambient * diffuseMap * attenuation;

    // ----- Diffuse lighting -----
    vec3 lightDir = normalize(light.position - FragPos);
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseMap *attenuation;

    // ----- Specular lighting -----
    vec3 viewDir = normalize(camPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = light.specular * spec * specularMap * attenuation;

    // ----- Result -----
    return vec4(vec3(ambient + diffuse + specular), alpha);
}

vec4 SpotLightColor( Light light, vec3 diffuseMap, vec3 specularMap, float shininess, float alpha )
{
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    // ----- Ambient lighting -----
    vec3 ambient = light.ambient * diffuseMap * attenuation;

    // ----- Diffuse lighting -----
    vec3 lightDir = normalize(light.position - FragPos);
    float theta = dot(lightDir, normalize(light.direction));

    if(theta < light.outerCutOff) return vec4(ambient, 1.0);

    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseMap * attenuation * intensity;

    // ----- Specular lighting -----
    vec3 viewDir = normalize(camPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = light.specular * spec * specularMap * attenuation * intensity;

    // ----- Result -----
    return vec4(vec3(ambient + diffuse + specular), alpha);
}

vec4 getFragColor_allInOne( Light light, vec3 diffuseMap, vec3 specularMap, float shininess, float alpha )
{
    float attenuation;
    if(light.lightType != 0)
    {
        float distance = length(light.position - FragPos);
        attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    }

    // ----- Ambient lighting -----
    vec3 ambient = light.ambient * diffuseMap;
    if(light.lightType != 0) ambient *= attenuation;

    // ----- Diffuse lighting -----
    vec3 lightDir;
    float intensity;
    if     (light.lightType == 0) lightDir = normalize(light.direction);            // directional light
    else if(light.lightType == 1) lightDir = normalize(light.position - FragPos);   // point light
    else if(light.lightType == 2)                                                   // spot light
    {
        lightDir = normalize(light.position - FragPos);
        float theta = dot(lightDir, normalize(light.direction));
        if(theta < light.outerCutOff)
            return vec4(ambient, 1.0);
        float epsilon = light.cutOff - light.outerCutOff;
        intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    }

    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseMap;
    if(light.lightType == 1) diffuse *= attenuation;
    if(light.lightType == 2) diffuse *= attenuation * intensity;

    // ----- Specular lighting -----
    vec3 viewDir = normalize(camPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = light.specular * spec * specularMap;
    if(light.lightType == 1) specular *= attenuation;
    if(light.lightType == 2) specular *= attenuation * intensity;

    // ----- Result -----
    return vec4(vec3(ambient + diffuse + specular), alpha);
}
