#ifndef SHADER_HPP
#define SHADER_HPP

#ifdef IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <GL/glew.h>
#elif IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <glad/glad.h>
#endif
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

/// Compiles and uses a GPU program. Sets uniforms.
class Shader
{
    void checkCompileErrors(unsigned int shaderID, std::string type);

public:
    unsigned int ID;        ///< Program identifier

    /*
    *   @brief Constructor. Creates a program, so the program's identifier is defined
    *   @param vertexPath Path for the vertex shader file
    *   @param fragmentPath Path for the fragment shader file
    */
    Shader(const char *vertexPath, const char *fragmentPath);
    void UseProgram();                                          ///< Use the program: Make it active in the current context

    // Uniforms ------------------------------
    void setBool (const std::string &name, bool value) const;                         ///< Boolean uniform creator (converted to int). 
    void setInt  (const std::string &name, int value) const;                          ///< Integer uniform creator
    void setFloat(const std::string &name, float value) const;                        ///< Float uniform creator
    void setVec2 (const std::string &name, const glm::vec2 &value) const;             ///< Vec2 uniform creator (from a vec2 argument)
    void setVec2 (const std::string &name, float x, float y) const;                   ///< Vec2 uniform creator (from 2 floats)
    void setVec3 (const std::string &name, const glm::vec3 &value) const;             ///< Vec3 uniform creator (from a vec3 argument)
    void setVec3 (const std::string &name, float x, float y, float z) const;          ///< Vec3 uniform creator (from 3 floats)
    void setVec4 (const std::string &name, const glm::vec4 &value) const;             ///< Vec4 uniform creator (from a vec4 argument)
    void setVec4 (const std::string &name, float x, float y, float z, float w) const; ///< Vec4 uniform creator (from 4 floats)
    void setMat2 (const std::string &name, const glm::mat2 &mat) const;               ///< Mat2 uniform creator
    void setMat3 (const std::string &name, const glm::mat3 &mat) const;               ///< Mat3 uniform creator
    void setMat4 (const std::string &name, const glm::mat4 &mat) const;               ///< Mat4 uniform creator
};

#endif
