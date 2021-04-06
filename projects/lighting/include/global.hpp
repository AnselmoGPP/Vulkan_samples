#ifndef GLOBAL_DATA_HPP
#define GLOBAL_DATA_HPP

#include "camera.hpp"
#include "auxiliar.hpp"
#include "geometry.hpp"

// Settings (typedef and global data section)

// camera --------------------
Camera cam(glm::vec3(128.0f, -30.0f, 150.0f));
float lastX =  SCR_WIDTH  / 2.0;
float lastY =  SCR_HEIGHT / 2.0;
bool firstMouse = true;
bool LMBpressed = false;
bool mouseOverGUI = false;

// timing --------------------
timerSet timer(30);

// Terrain data --------------------
noiseSet noise;
terrainGenerator terrain(noise, 0, 0, 1, 256, 256);
bool newTerrain = true;
float seaLevel = 0;

// Lighting --------------------

enum lightCaster { directional, point, spot };

struct light
{
    /// Constructor
    light(lightCaster type, glm::vec3 pos, glm::vec3 dir, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float cons, float line, float quad, float cut, float outerCut)
        : lightType(type), position(pos), direction(dir), ambient(amb), diffuse(diff), specular(spec), constant(cons), linear(line), quadratic(quad), cutOff(cut), outerCutOff(outerCut) { }

    lightCaster lightType;    ///< Type of light: Direction, point, spot
    glm::vec3   position;     ///< Light source position (point/spot light)
    glm::vec3   direction;    ///< direction from a fragment to the lightsource (directional light)

    glm::vec3   ambient;      ///< Ambient minimum possible light. Zero point light
    glm::vec3   diffuse;      ///< Light color
    glm::vec3   specular;     ///< Specular value (usually, it's 1, and the material determines its own specular value)

    float       constant;     ///< Attenuation constant factor (point/spot light)
    float       linear;       ///< Attenuation linear quoeficient (point/spot light)
    float       quadratic;    ///< Attenuation quadratic quoeficient (point/spot light)

    float       cutOff;      ///< Maximum angle (cosine). Everything outside is not lit (spot light)
    float       outerCutOff; ///< Smooth edges will be computed for the area between cutOff and outerCutOff (cosine)
};

light sun(directional,
          glm::vec3(-557.f, 577.f, 577.f),
          glm::vec3(-0.57735f, 0.57735f, 0.57735f),
          glm::vec3(0.1f),
          glm::vec3(1.f),
          glm::vec3(1.f),
          1.0f, 0.0014f, 0.000007f,
          glm::cos(glm::radians(12.5f)),
          glm::cos(glm::radians(14.5f)) );
/*
    Usual attenuation values:
     |Range|Constant|Linear|Cuadratic|
      3250  1.0      0.0014  0.000007
      600   1.0      0.007   0.0002
      325   1.0      0.014   0.0007
      200   1.0      0.022   0.0019
      160   1.0      0.027   0.0028
      100   1.0      0.045   0.0075
      65    1.0      0.07    0.017
      50    1.0      0.09    0.032
      32    1.0      0.14    0.07
      20    1.0      0.22    0.20
      13    1.0      0.35    0.44
      7     1.0      0.7     1.8
 */


// Materials --------------------

struct material
{
    material(float shin) : shininess(shin) { }
    material(glm::vec3 diff, glm::vec3 spec, float shin) : diffuse(diff), specular(spec), shininess(shin) { }

    unsigned diffuseT;      ///< Diffuse texture
    glm::vec3 diffuse;      ///< Object color
    unsigned specularT;     ///< Specular texture
    glm::vec3 specular;     ///< Less == More diffused reflection
    float shininess;        ///< More == Smaller reflection
};

material water(glm::vec3(0.1f, 0.1f, 0.8f),
               glm::vec3(0.5),
               32 );

material grass(glm::vec3(0.1f, 0.6f, 0.1f),
               glm::vec3(0.5),
               32 );

material rock( glm::vec3(0.2f, 0.2f, 0.2f),
               glm::vec3(0.2),
               32 );

#endif
