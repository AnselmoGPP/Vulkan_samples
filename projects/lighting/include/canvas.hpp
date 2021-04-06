#ifndef CANVAS_HPP
#define CANVAS_HPP

#include <array>

/*
*	@brief Create VAO (Vertex Array Object)
*	@return VAO identifier
*/
unsigned int createVAO();

/*
*	@brief Create VBO (Vertex Buffer Object)
*	@param num_bytes Number of byte you want to pass
*	@param pointerToData 
*	@param GL_XXX_DRAW
*	@return VBO identifier
*/
unsigned int createVBO(unsigned long num_bytes, void* pointerToData, int GL_XXX_DRAW);

/*
*	@brief Create EBO (Element Buffer Object)
*	@param num_bytes Size in bytes
*	@param pointerToData Pointer to the data
*	@param GL_XXX_DRAW Usage enum. Options: GL_DYNAMIC_DRAW, GL_STATIC_DRAW, GL_STREAM_DRAW
*	@return EBO identifier
*/
unsigned int createEBO(unsigned long num_bytes, void* pointerToData, int GL_XXX_DRAW);

/*
*	@brief Configure VAO (using VBO and EBO)
*	@param VAO to configure
*	@param VBO
*	@param EBO
*	@param sizes Array with the number of elements that contained in each attribute. Example: vertex is 3 floats, indice is 2 int
*	@param numAtribs Number of attributes in the VBO. Example: Vertex + Color + Texture coordinates + Normals = 4
*/
void configVAO(unsigned VAO, unsigned VBO, unsigned EBO, int *sizes, unsigned numAtribs);

/*
*	@brief Configure VAO (using VBO)
*	@param VAO The VAO to configure
*	@param VBO The VBO to set to our VAO
*	@param sizes Array with the number of elements that contained in each attribute. Example: vertex is 3 floats, indice is 2 int
*	@param numAtribs Number of attributes in the VBO. Example: Vertex + Color + Texture coordinates + Normals = 4
*/
void configVAO(unsigned VAO, unsigned VBO, int* sizes, unsigned numAtribs);

/*
*	@brief Get a 2D texture from a file
*	@param fileAddress Address of the file containing the texture
*	@param internalFormat Format of the texture in the file. Options: GL_RGB, GL_RGBA
*	@return Texture identifier
*/
unsigned createTexture2D(const char *fileAddress, int internalFormat);

#endif
