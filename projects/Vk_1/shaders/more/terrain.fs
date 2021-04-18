// TERRAIN

#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
//in vec3 ourColor;

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
uniform Material snow;
uniform Material sand;
uniform Material plainSand;

uniform vec4 skyColor;
uniform float fogMaxSquareRadius;
uniform float fogMinSquareRadius;

uniform vec3 camPos;

vec4 getTerrainTexture_GrassRock( vec4 fragment );
vec4 getTerrainTexture_Desert( vec4 fragment );
vec4 applyFog( vec4 fragment );


void main()
{
    vec4 result;

    //result = getTerrainTexture_GrassRock(result);
    result = getTerrainTexture_Desert(result);

    result = applyFog(result);

    FragColor = result;
}


vec4 DirectionalLightColor( Light light, vec3 diffuseMap, vec3 specularMap, float shininess, float alpha );
vec4 PointLightColor      ( Light light, vec3 diffuseMap, vec3 specularMap, float shininess, float alpha );
vec4 SpotLightColor       ( Light light, vec3 diffuseMap, vec3 specularMap, float shininess, float alpha );


// Apply the lighting type you want to a fragment
vec4 getFragColor( Light light, vec3 diffuseMap, vec3 specularMap, float shininess, float alpha )
{
    if(light.lightType == 0) return DirectionalLightColor( light, diffuseMap, specularMap, shininess, alpha );
    if(light.lightType == 1) return PointLightColor      ( light, diffuseMap, specularMap, shininess, alpha );
    if(light.lightType == 2) return SpotLightColor       ( light, diffuseMap, specularMap, shininess, alpha );
}


// Get the texture for the given fragment (depends upon the slope and lighting)
vec4 getTerrainTexture_Desert( vec4 fragment)
{
    float slopeThreshold = 0.3;           // sand-plainSand slope threshold
    float mixRange       = 0.1;           // threshold mixing range (slope range)
    float dtf            = 30;            // sand texture factor
    float ptf            = 30;            // plainSand texture factor

    float slope = dot( normalize(Normal), normalize(vec3(Normal.x, Normal.y, 0.0)) );

    // >>> DESERT
    if (slope < slopeThreshold - mixRange)
        fragment = getFragColor( sun, vec3(texture(sand.diffuseT, TexCoord/dtf)), vec3(texture(sand.specularT, TexCoord/dtf)), sand.shininess, 1.0);

    // >>> PLAIN
    else if(slope > slopeThreshold + mixRange)
        fragment = getFragColor( sun, vec3(texture(plainSand.diffuseT, TexCoord/ptf)), vec3(texture(plainSand.specularT, TexCoord/ptf)), plainSand.shininess, 1.0);

    // >>> MIXTURE (DESERT + PLAIN)
    else if(slope >= slopeThreshold - mixRange && slope <= slopeThreshold + mixRange)
    {
        vec4 plainFrag  = getFragColor( sun, vec3(texture(plainSand.diffuseT, TexCoord/dtf)), vec3(texture(plainSand.specularT, TexCoord/dtf)), plainSand.shininess, 1.0 );
        vec4 sandFrag = getFragColor( sun, vec3(texture(sand.diffuseT, TexCoord/ptf)), vec3(texture(sand.specularT, TexCoord/ptf)), sand.shininess, 1.0 );

        float ratio    = (slope - (slopeThreshold - mixRange)) / (2 * mixRange);
        vec3 mixGround = plainFrag.xyz * ratio + sandFrag.xyz * (1-ratio);

        fragment = vec4(mixGround, 1.0);
    }

    return fragment;
}


// Get the texture for the given fragment (depends upon the slope and lighting)
vec4 getTerrainTexture_GrassRock( vec4 fragment )
{
    float slopeThreshold = 0.5;           // grass-rock slope threshold
    float mixRange       = 0.05;          // threshold mixing range (slope range)
    float rtf            = 30;            // rock texture factor
    float gtf            = 20;            // grass texture factor

    float maxSnowLevel   = 80;            // maximum snow height (up from here, there's only snow within the maxSnowSlopw)
    float minSnowLevel   = 50;            // minimum snow height (down from here, there's zero snow)
    float maxSnowSlope   = 0.90;          // maximum slope where snow can rest
    float snowSlope      = maxSnowSlope * ( (FragPos.z - minSnowLevel) / (maxSnowLevel - minSnowLevel) );
    float mixSnowRange   = 0.1;           // threshold mixing range (slope range)

    if(snowSlope > maxSnowSlope) snowSlope = maxSnowSlope;
    float slope = dot( normalize(Normal), normalize(vec3(Normal.x, Normal.y, 0.0)) );

    // >>> SNOW
    if(slope < snowSlope)
    {
        vec4 snowFrag = getFragColor( sun, snow.diffuse, snow.specular, snow.shininess, 1.0 );

        // >>> MIXTURE (SNOW + GRASS + ROCK)
        if(slope > (snowSlope - mixSnowRange) && slope < snowSlope)
        {
            vec4 rockFrag  = getFragColor( sun, vec3(texture(rock.diffuseT, TexCoord/rtf)), vec3(texture(rock.specularT, TexCoord/rtf)), rock.shininess, 1.0 );
            vec4 grassFrag = getFragColor( sun, vec3(texture(grass.diffuseT, TexCoord/gtf)), vec3(texture(grass.specularT, TexCoord/gtf)), grass.shininess, 1.0 );
            float ratio    = (slope - (slopeThreshold - mixRange)) / (2 * mixRange);
            if(ratio < 0) ratio = 0;
            else if(ratio > 1) ratio = 1;
            vec3 mixGround = rockFrag.xyz * ratio + grassFrag.xyz * (1-ratio);

            ratio      = (snowSlope - slope) / mixSnowRange;
            if(ratio < 0) ratio = 0;
            else if (ratio > 1) ratio = 1;
            vec3 mix   = mixGround.xyz * (1-ratio) + snowFrag.xyz * ratio;
            snowFrag   = vec4( mix, 1.0 );
        }

        fragment = snowFrag;
    }

    // >>> GRASS
    else if (slope < slopeThreshold - mixRange)
        fragment = getFragColor( sun, vec3(texture(grass.diffuseT, TexCoord/gtf)), vec3(texture(grass.specularT, TexCoord/gtf)), grass.shininess, 1.0 );

    // >>> ROCK
    else if(slope > slopeThreshold + mixRange)
        fragment = getFragColor( sun, vec3(texture(rock.diffuseT, TexCoord/rtf)), vec3(texture(rock.specularT, TexCoord/rtf)), rock.shininess, 1.0 );

    // >>> MIXTURE (GRASS + ROCK)
    else if(slope >= slopeThreshold - mixRange && slope <= slopeThreshold + mixRange)
    {
        vec4 rockFrag  = getFragColor( sun, vec3(texture(rock.diffuseT, TexCoord/rtf)), vec3(texture(rock.specularT, TexCoord/rtf)), rock.shininess, 1.0 );
        vec4 grassFrag = getFragColor( sun, vec3(texture(grass.diffuseT, TexCoord/gtf)), vec3(texture(grass.specularT, TexCoord/gtf)), grass.shininess, 1.0 );

        float ratio    = (slope - (slopeThreshold - mixRange)) / (2 * mixRange);
        vec3 mixGround = rockFrag.xyz * ratio + grassFrag.xyz * (1-ratio);

        fragment = vec4(mixGround, 1.0);
    }

    return fragment;
}


// Apply fog (skyColor) to a fragment
vec4 applyFog(vec4 fragment)
{
    float squareDistance = (FragPos.x - camPos.x) * (FragPos.x - camPos.x) +
                           (FragPos.y - camPos.y) * (FragPos.y - camPos.y) +
                           (FragPos.z - camPos.z) * (FragPos.z - camPos.z);

    if(squareDistance > fogMinSquareRadius)
        if(squareDistance > fogMaxSquareRadius)
            fragment = skyColor;
        else
    {
        float ratio  = (squareDistance - fogMinSquareRadius) / (fogMaxSquareRadius - fogMinSquareRadius);
        fragment = vec4(fragment.xyz * (1-ratio) + skyColor.xyz * ratio, fragment.a);
    }

    return fragment;
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
