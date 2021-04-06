
#include <iostream>

#ifdef IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "GL/glew.h"
#elif IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "glad/glad.h"
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "canvas.hpp"

unsigned int createVAO()
{
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    //glBindVertexArray(VAO);

    //glBindVertexArray(0);
    return VAO;
}

unsigned int createVBO(unsigned long num_bytes, void* pointerToData, int GL_XXX_DRAW)
{
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, num_bytes, pointerToData, GL_XXX_DRAW);
/*
    if( num_bytes == (sizeof(float) * 2601 * 8) )//ans2
    {
        std::cout << "[" << num_bytes/4 << "]" << std::endl;
        for(int i = 0; i < 10; i++)
        {
            std::cout << i << ": " << ((float*)pointerToData)[i*8    ] << ", "
                                   << ((float*)pointerToData)[i*8 + 1] << ", "
                                   << ((float*)pointerToData)[i*8 + 2] << std::endl;
        }
    }
*/
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return VBO;
}

unsigned int createEBO(unsigned long num_bytes, void* pointerToData, int GL_XXX_DRAW)
{
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_bytes, pointerToData, GL_XXX_DRAW);
/*
    if( num_bytes == (sizeof(float) * 15000) )//ans2
    {
        std::cout << "[" << num_bytes/4 << "]" << std::endl;
        for(int i = 0; i < 15000/3; i++)
        {
            std::cout << i << ": " << ((unsigned*)pointerToData)[i*3    ] << ", "
                                   << ((unsigned*)pointerToData)[i*3 + 1] << ", "
                                   << ((unsigned*)pointerToData)[i*3 + 2] << std::endl;
        }
    }
*/
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return EBO;
}

void configVAO(unsigned VAO, int *sizes, unsigned numAtribs)
{
    // Get stride
    int totalSize = 0;
    for(unsigned i = 0; i < numAtribs; i++) totalSize += sizes[i];
    int stride = totalSize * sizeof(float);

    glBindVertexArray(VAO);

    // Specify pointers
    int pointer = 0;
    for(unsigned i = 0; i < numAtribs; i++)
    {
        glVertexAttribPointer( i, sizes[i], GL_FLOAT, GL_FALSE, stride, (void *)(pointer * sizeof(float)) );
        glEnableVertexAttribArray(i);

        pointer += sizes[i];
    }

    glBindVertexArray(0);
}

void configVAO(unsigned VAO, unsigned VBO, int* sizes, unsigned numAtribs)
{
    // Get stride
    int totalSize = 0;
    for (unsigned i = 0; i < numAtribs; i++) totalSize += sizes[i];
    int stride = totalSize * sizeof(float);

    // Bind buffers
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Specify pointers
    int pointer = 0;
    for (unsigned i = 0; i < numAtribs; i++)
    {
        glVertexAttribPointer(i, sizes[i], GL_FLOAT, GL_FALSE, stride, (void*)(pointer * sizeof(float)));
        glEnableVertexAttribArray(i);

        pointer += sizes[i];
    }

    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);           // unbind VBO (not usual)
    glBindVertexArray(0);                       // unbind VAO (not usual)
}

void configVAO(unsigned VAO, unsigned VBO, unsigned EBO, int *sizes, unsigned numAtribs, bool bindVAO)
{
    // Get stride
    int totalSize = 0;
    for(unsigned i = 0; i < numAtribs; i++) totalSize += sizes[i];
    int stride = totalSize * sizeof(float);

    // Bind buffers
    if(bindVAO) glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Specify pointers
    int pointer = 0;
    for(unsigned i = 0; i < numAtribs; i++)
    {
        glVertexAttribPointer( i, sizes[i], GL_FLOAT, GL_FALSE, stride, (void *)(pointer * sizeof(float)) );
        glEnableVertexAttribArray(i);

        pointer += sizes[i];
    }

    // Unbind buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);           // unbind VBO (not usual)
    if(bindVAO) glBindVertexArray(0);           // unbind VAO (not usual)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);   // unbind EBO
}

unsigned createTexture2D(const char *fileAddress, int internalFormat)
{
    stbi_set_flip_vertically_on_load(true);

    unsigned texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);               // GL_REPEAT  GL_MIRRORED_REPEAT  GL_CLAMP_TO_EDGE  GL_CLAMP_TO_BORDER

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // Texture filtering (?)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);           // GL_LINEAR  GL_NEAREST

    int width, height, numberChannels;
    unsigned char *image = stbi_load(fileAddress, &width, &height, &numberChannels, 0);

    if(image)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, (numberChannels == 4? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else std::cout << "Failed to load texture" << std::endl;

    stbi_image_free(image);

    return texture;
}






