/*
 *  https://stackoverflow.com/questions/57336940/how-to-glutdisplayfunc-glutmainloop-in-glfw
 */

// Includes --------------------

#include <iostream>

#ifdef IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "GL/glew.h"
#elif IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "glad/glad.h"
#endif
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "auxiliar.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "geometry.hpp"
#include "myGUI.hpp"
#include "canvas.hpp"
#include "global.hpp"

// Function declarations --------------------

void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);

void GUI_terrainConfig();
void printOGLdata();

void setUniformsTerrain(Shader &program, unsigned diffuseMap, unsigned especularMap, unsigned diffuseMap2, unsigned especularMap2);
void setUniformsAxis(Shader& program);
void setUniformsWater(Shader& program);
void setUniformsLightSource(Shader& program);

// Function definitions --------------------

int main()
{
    // glfw: initialize and configure
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 1);                                // antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // To make MacOS happy; should not be needed
#endif

    // ----- GLFW window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Simulator", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window (note: Intel GPUs are not 3.3 compatible)" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // ----- Event callbacks and control handling
    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  // DISABLED, HIDDEN, NORMAL
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);        // Sticky keys: Make sure that any pressed key is captured

    // ----- Load OGL function pointers with GLEW or GLAD
    #ifdef IMGUI_IMPL_OPENGL_LOADER_GLEW
        glewExperimental = true;        // Needed for core profile (no more from GLEW 2.0)
        GLenum glewErr = glewInit();
        if (glewErr != GLEW_OK)
        {
            std::cerr << "GLEW error (" << glewErr << "): " << glewGetErrorString(glewErr) << "\n" << std::endl;
            glfwTerminate();
            return -1;
        }
    #elif IMGUI_IMPL_OPENGL_LOADER_GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "GLAD initialization failed" << std::endl;
            return -1;
        }
    #endif

    // ----- OGL general options
    printOGLdata();

    glEnable(GL_DEPTH_TEST);                            // Depth test (draw what is the front)    Another option: glDepthFunc(GL_LESS);  // Accept fragment if it's closer to the camera than the former one

    glFrontFace(GL_CCW);                                // Front face is drawn counterclock wise

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);        // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);        // Back to default

    glEnable(GL_BLEND);                                 // Enable transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Enable transparency

    glEnable(GL_CULL_FACE);

    /*
    // Point parameters
    glEnable(GL_PROGRAM_POINT_SIZE);			// Enable GL_POINTS
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);		// Enable gl_PointSize in the vertex shader
    glEnable(GL_POINT_SMOOTH);					// For circular points (GPU implementation dependent)
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);	// For circular points (GPU implementation dependent)
    glPointSize(5.0);							// GL_POINT_SIZE_MAX is GPU implementation dependent

    // Lines parameters
    glLineWidth(2.0);
    GLfloat lineWidthRange[2];                  // GPU implementation dependent
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
    */

    // ----- Set up vertex data, buffers, and configure vertex attributes

    // >>> Terrain

    Shader terrProgram(
        "../../../projects/lighting/shaders/vertexShader.vs",
        "../../../projects/lighting/shaders/fragmentShader.fs");

    unsigned int VAO = createVAO();
    unsigned int VBO = createVBO(sizeof(float)*terrain.getNumVertex()*11, terrain.vertex, GL_STATIC_DRAW);
    unsigned int EBO = createEBO(sizeof(unsigned)*terrain.getNumIndices(), terrain.indices, GL_STATIC_DRAW);

    int sizesAtribsTerrain[4] = { 3, 3, 2, 3 };
    configVAO(VAO, VBO, EBO, sizesAtribsTerrain, 4);

    grass.diffuseT  = createTexture2D("../../../textures/grass.png", GL_RGB);
    grass.specularT = createTexture2D("../../../textures/grass_specular.png", GL_RGB);
    rock.diffuseT   = createTexture2D("../../../textures/rock.jpg", GL_RGB);
    rock.specularT  = createTexture2D("../../../textures/rock_specular.jpg", GL_RGB);
    terrProgram.UseProgram();
    terrProgram.setInt("grass.diffuseT",  0);      // Tell OGL for each sampler to which texture unit it belongs to (only has to be done once)
    terrProgram.setInt("grass.specularT", 1);
    terrProgram.setInt("rock.diffuseT",   2);
    terrProgram.setInt("rock.specularT",  3);

    // >>> Axis

    float coordSys[6][6];
    fillAxis(coordSys, 256);

    Shader axisProg(
        "../../../projects/lighting/shaders/vertexVec3_colorVec3.vs",
        "../../../projects/lighting/shaders/vertexVec3_colorVec3.fs");

    unsigned axisVAO = createVAO();
    unsigned axisVBO = createVBO(sizeof(float) * 6 * 6, &coordSys[0][0], GL_STATIC_DRAW);

    int sizesAtribsAxis[2] = { 3, 3 };
    configVAO(axisVAO, axisVBO, sizesAtribsAxis, 2);

    // >>> Light source (icosahedron)
    Icosahedron icos;

    Shader lsProg(
        "../../../projects/lighting/shaders/vertexVec3_colorUnifVec4.vs",
        "../../../projects/lighting/shaders/vertexVec3_colorUnifVec4.fs");

    unsigned lsVAO = createVAO();
    unsigned lsVBO = createVBO(sizeof(float) * icos.numVertices, icos.vertices, GL_STATIC_DRAW);
    unsigned int lsEBO = createEBO(sizeof(unsigned)*icos.numIndices, icos.indices, GL_STATIC_DRAW);

    int sizesAtribsLightSource[1] = { 3 };
    configVAO(lsVAO, lsVBO, lsEBO, sizesAtribsLightSource, 1);

    // >>> Water

    Shader waterProg(
        "../../../projects/lighting/shaders/vertexVec3_colorVec4_normalVec3.vs",
        "../../../projects/lighting/shaders/vertexVec3_colorVec4_normalVec3.fs");

    float water[6][10];
    fillSea(water, seaLevel, 0.8f, 0, 0, 127, 127);

    unsigned waterVAO = createVAO();
    unsigned waterVBO = createVBO(sizeof(float) * 6 * 10, &water[0][0], GL_STATIC_DRAW);

    int sizesAtribsWater[3] = { 3, 4, 3 };
    configVAO(waterVAO, waterVBO, sizesAtribsWater, 3);

    // ----- Load and create a texture
/*
    // Texture 2
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    image = stbi_load("../../../textures/note.png", &width, &height, &numberChannels, 0);
    if(image)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, (numberChannels == 4? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else std::cout << "Failed to load texture" << std::endl;

    stbi_image_free(image);
*/


    // ----- Other operations

    myGUI gui(window);

    timer.startTime();
    
    // >>>>> RENDER LOOP <<<<<

    while (!glfwWindowShouldClose(window))
    {
        timer.computeDeltaTime();
        
        processInput(window);

        std::string title = "Simulator (fps: " + std::to_string(timer.getFPS()) + ")";
        glfwSetWindowTitle(window, title.c_str());

        // RENDERING ------------------------
        // ----------------------------------

        glClearColor(0.0f, 0.24f, 0.39f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);           // GL_STENCIL_BUFFER_BIT
        
        // GUI
        gui.implement_NewFrame();
        GUI_terrainConfig();
        mouseOverGUI = gui.cursorOverGUI();

        // >>> Terrain
        if(newTerrain)
        {
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*terrain.getNumVertex()*11, terrain.vertex, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * terrain.getNumIndices(), terrain.indices, GL_STATIC_DRAW);

            newTerrain = false;
        }

        setUniformsTerrain(terrProgram, grass.diffuseT, grass.specularT, rock.diffuseT, rock.specularT);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, terrain.getNumIndices(), GL_UNSIGNED_INT, nullptr);    //glDrawArrays(GL_TRIANGLES, 0, 36);

        // >>> Water
        if(water[4][0] != terrain.getXside() || water[4][1] != terrain.getYside() || water[0][2] != seaLevel)
        {
            fillSea(water, seaLevel, 0.8f, 0., 0., terrain.getXside()-1, terrain.getYside()-1);

            glBindVertexArray(waterVAO);
            glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*10, &water[0][0], GL_STATIC_DRAW);
        }

        setUniformsWater(waterProg);
        glBindVertexArray(waterVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // >>> Axis
        setUniformsAxis(axisProg);
        glBindVertexArray(axisVAO);
        glDrawArrays(GL_LINES, 0, 6);

        // >>> Light source
        setUniformsLightSource(lsProg);
        glBindVertexArray(lsVAO);
        glDrawElements(GL_TRIANGLES, icos.numIndices, GL_UNSIGNED_INT, nullptr);

        // GUI
        gui.render();

        // ----------------------------------
        // ----------------------------------

        timer.printTimeData();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Render loop End


    // ----- De-allocate all resources

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(terrProgram.ID);

    glDeleteVertexArrays(1, &axisVAO);
    glDeleteBuffers(1, &axisVBO);
    glDeleteProgram(axisProg.ID);

    glDeleteVertexArrays(1, &waterVAO);
    glDeleteBuffers(1, &waterVBO);
    glDeleteProgram(waterProg.ID);

    glDeleteVertexArrays(1, &lsVAO);
    glDeleteBuffers(1, &lsVBO);
    glDeleteBuffers(1, &lsEBO);
    glDeleteProgram(lsProg.ID);
    
    // GUI
    gui.cleanup();
    
    glfwTerminate();

    return 0;
}

// Event handling --------------------------------------------------------------------

// GLFW: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
    //glfwGetFramebufferSize(window, &width, &height);  // Get viewport size from GLFW
    glViewport(0, 0, width, height);                    // Tell OGL the viewport size

    // projection adjustments
    cam.width = width;
    cam.height = height;
}

// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Get cameraPos from keys
    float cameraSpeed = 2.5 * timer.getDeltaTime();
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam.ProcessKeyboard(FORWARD, timer.getDeltaTime());
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam.ProcessKeyboard(BACKWARD, timer.getDeltaTime());
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam.ProcessKeyboard(LEFT, timer.getDeltaTime());
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam.ProcessKeyboard(RIGHT, timer.getDeltaTime());
}

// Get cameraFront from the mouse
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (LMBpressed)
    {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = lastX - xpos;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        cam.ProcessMouseMovement(xoffset, yoffset, 1);
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (!mouseOverGUI)
        cam.ProcessMouseScroll(yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (!mouseOverGUI)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            LMBpressed = true;
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            LMBpressed = false;
            firstMouse = true;
        }
    }
}

// Others ----------------------------------------------------------------------------

void printOGLdata()
{
    int maxNumberAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxNumberAttributes);

    std::cout << "-------------------- \n" <<
                 "OpenGL data: " <<
                 "\n    - Version: "                    << glGetString(GL_VERSION) <<
                 "\n    - Vendor: "                     << glGetString(GL_VENDOR) <<
                 "\n    - Renderer: "                   << glGetString(GL_RENDERER) <<
                 "\n    - GLSL version: "               << glGetString(GL_SHADING_LANGUAGE_VERSION) <<
                 "\n    - Max. attributes supported: "  << maxNumberAttributes << std::endl <<
                 "-------------------- \n" << std::endl;
}

void GUI_terrainConfig()
{
    int dimensionX      = terrain.getXside();
    int dimensionY      = terrain.getYside();

    const char* noiseTypeString[6] = { "OpenSimplex2", "OpenSimplex2S", "Cellular", "Perlin", "ValueCubic", "Value" };
    int noiseType       = noise.getNoiseType();
    int numOctaves      = noise.getNumOctaves();
    float lacunarity    = noise.getLacunarity();
    float persistance   = noise.getPersistance();
    float scale         = noise.getScale();
    int multiplier      = noise.getMultiplier();
    int curveDegree     = noise.getCurveDegree();
    int offsetX         = noise.getOffsetX();
    int offsetY         = noise.getOffsetY();
    int seed            = noise.getSeed();

    // Window
    ImGui::Begin("Noise configuration");
    //ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::Text("Map dimensions:");
    ImGui::SliderInt("X dimension", &dimensionX, 1, 256);
    ImGui::SliderInt("Y dimension", &dimensionY, 1, 256);

    ImGui::Text("Noise configuration: ");
    ImGui::Combo("Noise type", &noiseType, noiseTypeString, IM_ARRAYSIZE(noiseTypeString));
    ImGui::SliderInt("Octaves", &numOctaves, 1, 10);
    ImGui::SliderFloat("Lacunarity", &lacunarity, 1.0f, 2.5f);
    ImGui::SliderFloat("Persistance", &persistance, 0.0f, 1.0f);
    ImGui::SliderFloat("Scale", &scale, 0.01f, 1.0f);
    ImGui::SliderInt("Multiplier", &multiplier, 1, 200);
    ImGui::SliderInt("Curve degree", &curveDegree, 0, 10);

    ImGui::Text("Map selection: ");
    ImGui::SliderInt("Seed", &seed, 0, 100000);
    ImGui::SliderInt("X offset", &offsetX, -500, 500);
    ImGui::SliderInt("Y offset", &offsetY, -500, 500);

    ImGui::Text("Water: ");
    ImGui::SliderFloat("Sea level", &seaLevel, 0, 128);

    ImGui::Text("Camera: ");
    ImGui::SliderFloat("Speed", &cam.MovementSpeed, 0, 200);

    //ImGui::ColorEdit3("clear color", (float*)&clear_color);
    //if (ImGui::Button("Button")) counter++;  ImGui::SameLine();  ImGui::Text("counter = %d", counter);
    //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    // Types conversion
    noiseSet newNoise((unsigned)numOctaves, lacunarity, persistance, scale, multiplier, curveDegree, offsetX, offsetY, (FastNoiseLite::NoiseType)noiseType, false, (unsigned)seed);

    if( noise != newNoise || dimensionX != terrain.getXside() ||  dimensionY != terrain.getYside() )
    {
        noise = newNoise;
        terrain.computeTerrain(noise, 0, 0, 1, dimensionX, dimensionY);
        newTerrain = true;
    }
}

void setUniformsTerrain(Shader &program, unsigned diffuseMap1, unsigned especularMap1, unsigned diffuseMap2, unsigned especularMap2)
{
    program.UseProgram();

    // Fragment shader uniforms
    program.setInt  ("sun.lightType",   sun.lightType);
    program.setVec3 ("sun.position",    sun.position);
    program.setVec3 ("sun.direction",   sun.direction);
    program.setVec3 ("sun.ambient",     sun.ambient);
    program.setVec3 ("sun.diffuse",     sun.diffuse);
    program.setVec3 ("sun.specular",    sun.specular);
    program.setFloat("sun.constant",    sun.constant);
    program.setFloat("sun.linear",      sun.linear);
    program.setFloat("sun.quadratic",   sun.quadratic);
    program.setFloat("sun.cutOff",      sun.cutOff);
    program.setFloat("sun.outerCutOff", sun.outerCutOff);

    program.setVec3 ("grass.diffuse",   grass.diffuse);
    program.setVec3 ("grass.specular",  grass.specular);
    program.setFloat("grass.shininess", grass.shininess);

    program.setVec3 ("rock.diffuse",    rock.diffuse);
    program.setVec3 ("rock.specular",   rock.specular);
    program.setFloat("rock.shininess",  rock.shininess);

    program.setVec3 ("camPos",          cam.Position);

    // Textures uniforms
    glActiveTexture(GL_TEXTURE0);               // Bind textures on corresponding texture unit
    glBindTexture(GL_TEXTURE_2D, diffuseMap1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, especularMap1);
    glActiveTexture(GL_TEXTURE2);               // Bind textures on corresponding texture unit
    glBindTexture(GL_TEXTURE_2D, diffuseMap2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, especularMap2);

    // Vertex shader uniforms
    glm::mat4 projection = cam.GetProjectionMatrix();
    program.setMat4("projection", projection);

    glm::mat4 view = cam.GetViewMatrix();
    program.setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    program.setMat4("model", model);

    glm::mat3 normalMatrix = glm::mat3( glm::transpose(glm::inverse(model)) );      // Used when the model matrix applies non-uniform scaling (normal won't be scaled correctly). Otherwise, use glm::vec3(model)
    program.setMat3("normalMatrix", normalMatrix);
}

void setUniformsWater(Shader& program)
{
    program.UseProgram();

    // Fragment shader uniforms
    program.setInt("sun.lightType",  sun.lightType);
    program.setVec3("sun.position",  sun.position);
    program.setVec3("sun.direction", sun.direction);
    program.setVec3("sun.ambient",   sun.ambient);
    program.setVec3("sun.diffuse",   sun.diffuse);
    program.setVec3("sun.specular",  sun.specular);

    program.setVec3("water.diffuse", water.diffuse);
    program.setVec3("water.specular", water.specular);
    program.setFloat("water.shininess", water.shininess);

    program.setVec3("viewPos", cam.Position);

    // Vertex shader uniforms
    glm::mat4 projection = cam.GetProjectionMatrix();
    program.setMat4("projection", projection);

    glm::mat4 view = cam.GetViewMatrix();
    program.setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    program.setMat4("model", model);

    glm::mat3 normalMatrix = glm::mat3( glm::transpose(glm::inverse(model)) );      // Used when the model matrix applies non-uniform scaling (normal won't be scaled correctly). Otherwise, use glm::vec3(model)
    program.setMat3("normalMatrix", normalMatrix);
}

void setUniformsAxis(Shader& program)
{
    program.UseProgram();

    // Vertex shader uniforms
    glm::mat4 projection = cam.GetProjectionMatrix();
    program.setMat4("projection", projection);

    glm::mat4 view = cam.GetViewMatrix();
    program.setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    program.setMat4("model", model);
}

void setUniformsLightSource(Shader& program)
{
    program.UseProgram();

    // Fragment shader uniforms
    program.setVec4("lightColor", glm::vec4(sun.diffuse, 1.f));

    // Vertex shader uniforms
    glm::mat4 projection = cam.GetProjectionMatrix();
    program.setMat4("projection", projection);

    glm::mat4 view = cam.GetViewMatrix();
    program.setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sun.position);
    model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(50.0f, 50.0f, 50.0f));
    program.setMat4("model", model);
}
