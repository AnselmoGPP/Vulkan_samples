/*
 * [Example 1]: Create a billboard of a sun -----------------------------------------
 *
 *     sun.diffuseT  = createTexture2D((path_textures + "sun.jpg")         .c_str(), GL_RGB);   // 0
 *     sun.specularT = createTexture2D((path_textures + "sun_specular.jpg").c_str(), GL_RGB);   // 1
 *
 *     Shader sunProg( (path_shaders + "sun.vs").c_str(), (path_shaders + "sun.fs").c_str() );
 *
 *     unsigned sunVAO = createVAO();
 *     unsigned sunVBO = createVBO(sizeof(float)    * 4*5, vertexArray,  GL_STATIC_DRAW);
 *     unsigned sunEBO = createEBO(sizeof(unsigned) * 2*3, indicesArray, GL_STATIC_DRAW);
 *
 *     int sizesAtribsSun[2] = { 3, 2 };
 *     configVAO(sunVAO, sunVBO, sunEBO, sizesAtribsSun, 2);
 *
 *     sunProg.UseProgram();
 *     sunProg.setInt("sunTexture",  0);
 *     sunProg.setInt("sunSpecula",  1);
 *
 *     while("RENDER LOOP")
 *     {
 *         setUniformsSun(sunProg);            // Set uniforms
 *         glBindVertexArray(sunVAO);
 *         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sunEBO);
 *         glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, nullptr);
 *         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
 *     }
 *
 *     glDeleteVertexArrays(1, &sunVAO);
 *     glDeleteBuffers(1, &sunVBO);
 *     glDeleteBuffers(1, &sunEBO);
 *     glDeleteProgram(sunProg.ID);
 *
 * [Example 2]: Create two billboard of a sun using a single VAO --------------------
 *
 *     sun.diffuseT  = createTexture2D((path_textures + "sun.jpg")         .c_str(), GL_RGB);   // 0
 *     sun.specularT = createTexture2D((path_textures + "sun_specular.jpg").c_str(), GL_RGB);   // 1
 *
 *     Shader testProg( (path_shaders + "sun.vs").c_str(), (path_shaders + "sun.fs").c_str() );
 *
 *     unsigned VAO  = createVAO();
 *     unsigned VBO1 = createVBO(sizeof(float)    * 4*5, vertexArray1,  GL_STATIC_DRAW);
 *     unsigned VBO2 = createVBO(sizeof(float)    * 4*5, vertexArray2,  GL_STATIC_DRAW);
 *     unsigned EBO1 = createEBO(sizeof(unsigned) * 2*3, indicesArray1, GL_STATIC_DRAW);
 *     unsigned EBO2 = createEBO(sizeof(unsigned) * 2*3, indicesArray2, GL_STATIC_DRAW);
 *
 *     int testSizes[2] = { 3, 2 };     // Now, configVAO() is in the render-loop
 *
 *     testProg.UseProgram();
 *     sunProg.setInt("sunTexture",  0);
 *     sunProg.setInt("sunSpecula",  1);
 *
 *     while("RENDER LOOP")
 *     {
 *         setUniformsTest(testProg);           // Set uniforms
 *
 *         configVAO(testVAO, VBO1, EBO1, testSizes, 2);
 *         glBindVertexArray(testVAO);
 *         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, testEBO1);
 *         glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, nullptr);
 *
 *         configVAO(testVAO, VBO2, EBO2, testSizes, 2);
 *         glBindVertexArray(testVAO);
 *         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, testEBO2);
 *         glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, nullptr);
 *
 *         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
 *     }
 *
 *     glDeleteVertexArrays(1, &VAO);
 *     glDeleteBuffers(1, &VBO1);
 *     glDeleteBuffers(1, &EBO1);
 *     glDeleteBuffers(1, &VBO2);
 *     glDeleteBuffers(1, &EBO2);
 *     glDeleteProgram(sunProg.ID);
 *
 */

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
*	@param sizes Array with the number of elements that contained in each attribute. Example: vertex is 3 floats, indice is 2 int
*	@param numAtribs Number of attributes in the VBO. Example: Vertex + Color + Texture coordinates + Normals = 4
*/
void configVAO(unsigned VAO, int *sizes, unsigned numAtribs);

/*
*	@brief Configure VAO (using VBO)
*	@param VAO The VAO to configure
*	@param VBO The VBO to set to our VAO
*	@param sizes Array with the number of elements that contained in each attribute. Example: vertex is 3 floats, indice is 2 int
*	@param numAtribs Number of attributes in the VBO. Example: Vertex + Color + Texture coordinates + Normals = 4
*/
void configVAO(unsigned VAO, unsigned VBO, int* sizes, unsigned numAtribs);

/*
*	@brief Configure VAO (using VBO and EBO)
*	@param VAO to configure
*	@param VBO
*	@param EBO
*	@param sizes Array with the number of elements that contained in each attribute. Example: vertex is 3 floats, indice is 2 int
*	@param numAtribs Number of attributes in the VBO. Example: Vertex + Color + Texture coordinates + Normals = 4
*/
void configVAO(unsigned VAO, unsigned VBO, unsigned EBO, int *sizes, unsigned numAtribs, bool bindVAO = true);

/*
*	@brief Get a 2D texture from a file
*	@param fileAddress Address of the file containing the texture
*	@param internalFormat Format of the texture in the file. Options: GL_RGB, GL_RGBA
*	@return Texture identifier
*/
unsigned createTexture2D(const char *fileAddress, int internalFormat);

#endif
