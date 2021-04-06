#include "shader.hpp"

void Shader::checkCompileErrors(unsigned int shaderID, std::string type)
{
    int success;
    char infoLog[1024];

    if(type != "PROGRAM")
    {
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(shaderID, 1024, nullptr, infoLog);
            std::cout << type << "_SHADER_COMPILATION_ERROR \n" << infoLog <<
                         "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetProgramInfoLog(shaderID, 1024, nullptr, infoLog);
            std::cout << type << "_LINKING_ERROR \n" << infoLog <<
                         "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

Shader::Shader(const char *vertexPath, const char *fragmentPath)
{
    // 1) Retrieve the shaders source code the paths

    std::ifstream vertexFile;
    std::ifstream fragmentFile;
    std::string vertexString;
    std::string fragmentString;

    // Ensure ifstream objects can throw exceptions:
    vertexFile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
    fragmentFile.exceptions( std::ifstream::failbit | std::ifstream::badbit );

    try
    {
        vertexFile.open(vertexPath);
        fragmentFile.open(fragmentPath);

        std::stringstream vertexStream, fragmentStream;
        vertexStream << vertexFile.rdbuf();
        fragmentStream << fragmentFile.rdbuf();

        vertexFile.close();
        fragmentFile.close();

        vertexString = vertexStream.str();
        fragmentString = fragmentStream.str();
    }
    catch( std::ifstream::failure &e )
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    const char *vertexCode = vertexString.c_str();
    const char *fragmentCode = fragmentString.c_str();

    // 2) Compile the shaders and the program

    unsigned int vertexID, fragmentID;

    vertexID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexID, 1, &vertexCode, nullptr);
    glCompileShader(vertexID);
    checkCompileErrors(vertexID, "VERTEX");

    fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentID, 1, &fragmentCode, nullptr);
    glCompileShader(fragmentID);
    checkCompileErrors(fragmentID, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, vertexID);
    glAttachShader(ID, fragmentID);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(vertexID);
    glDeleteShader(fragmentID);
}

void Shader::UseProgram()
{
    glUseProgram(ID);
}

// Uniforms setting ---------------

void Shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string &name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string &name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
