/*
 *  https://stackoverflow.com/questions/57336940/how-to-glutdisplayfunc-glutmainloop-in-glfw
 *  http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
 */

// Macros -----------------------------------

//#define SINGLE_VAO        // 0.000802952 sec
#define MANY_VAO 1          // 0.000680759 sec
//#define IMGUI_IMPL_OPENGL_LOADER_GLAD 1

// Includes --------------------

#include <iostream>
#include <exception>

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
#include "world.hpp"
#include "timelib.hpp"

// Function declarations --------------------

void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);

void updateTerrain(unsigned int VAO, std::map<BinaryKey, unsigned int> &VBO, std::map<BinaryKey, unsigned int> &EBO);
void updateTerrain(std::map<BinaryKey, unsigned int> &VAO, std::map<BinaryKey, unsigned int> &VBO, std::map<BinaryKey, unsigned int> &EBO);
void GUI_terrainConfig(std::map<BinaryKey, unsigned int> &VAO, std::map<BinaryKey, unsigned int> &VBO, std::map<BinaryKey, unsigned int> &EBO);
void printOGLdata();

void setUniformsTerrain(Shader &program);
void setUniformsAxis(Shader& program);
void setUniformsWater(Shader& program);
void setUniformsSun(Shader& program, glm::vec3 direction, float sunFOV);
void setUniformsLightSource(Shader& program);
void setUniformsTest(Shader& program);

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

    // >>> Textures
    grass.diffuseT       = createTexture2D((path_textures + "grass.png").c_str(), GL_RGB);              // 0
    grass.specularT      = createTexture2D((path_textures + "grass_specular.png").c_str(), GL_RGB);     // 1
    rock.diffuseT        = createTexture2D((path_textures + "rock.jpg").c_str(), GL_RGB);               // 2
    rock.specularT       = createTexture2D((path_textures + "rock_specular.jpg").c_str(), GL_RGB);      // 3
    sand.diffuseT        = createTexture2D((path_textures + "sand.jpg").c_str(), GL_RGB);               // 4
    sand.specularT       = createTexture2D((path_textures + "sand_specular.jpg").c_str(), GL_RGB);      // 5
    plainSand.diffuseT   = createTexture2D((path_textures + "plainSand.jpg").c_str(), GL_RGB);          // 6
    plainSand.specularT  = createTexture2D((path_textures + "plainSand_specular.jpg").c_str(), GL_RGB); // 7
    sun.diffuseT         = createTexture2D((path_textures + "sun.png").c_str(), GL_RGBA);               // 8

    // >>> Terrain
    Shader terrProgram( (path_shaders + "terrain.vs").c_str(), (path_shaders + "terrain.fs").c_str() );

    worldChunks.updateVisibleChunks(cam.Position);

    #ifdef SINGLE_VAO
        unsigned VAO = createVAO();
    #elif MANY_VAO
        std::map<BinaryKey, unsigned int> VAO;
    #endif

    std::map<BinaryKey, unsigned int> VBO;
    std::map<BinaryKey, unsigned int> EBO;

    terrProgram.UseProgram();
    terrProgram.setInt("grass.diffuseT",      0);  // Tell OGL for each sampler to which texture unit it belongs to (only has to be done once)
    terrProgram.setInt("grass.specularT",     1);
    terrProgram.setInt("rock.diffuseT",       2);
    terrProgram.setInt("rock.specularT",      3);
    terrProgram.setInt("sand.diffuseT",       4);
    terrProgram.setInt("sand.specularT",      5);
    terrProgram.setInt("plainSand.diffuseT",  6);
    terrProgram.setInt("plainSand.specularT", 7);

    // >>> Axis

    float coordSys[6][6];
    fillAxis(coordSys, 256);

    Shader axisProg( (path_shaders + "axis.vs").c_str(), (path_shaders + "axis.fs").c_str() );

    unsigned axisVAO = createVAO();
    unsigned axisVBO = createVBO(sizeof(float) * 6 * 6, &coordSys[0][0], GL_STATIC_DRAW);

    int sizesAtribsAxis[2] = { 3, 3 };
    configVAO(axisVAO, axisVBO, sizesAtribsAxis, 2);

    // >>> Water

    Shader waterProg( (path_shaders + "sea.vs").c_str(), (path_shaders + "sea.fs").c_str() );

    float water[6][10];
    fillSea(water, seaLevel, 0.8f, 0, 0, 127, 127);

    unsigned waterVAO = createVAO();
    unsigned waterVBO = createVBO(sizeof(float) * 6 * 10, &water[0][0], GL_STATIC_DRAW);

    int sizesAtribsWater[3] = { 3, 4, 3 };
    configVAO(waterVAO, waterVBO, sizesAtribsWater, 3);

    // >>> Sun billboard (light source)

    float    sunVertex[4][5]  = { {-0.5f, -0.5f, 0.0f, 0.f, 0.f}, { 0.5f, -0.5f, 0.0f, 1.f, 0.f}, { 0.5f,  0.5f, 0.0f, 1.f, 1.f}, {-0.5f,  0.5f, 0.0f, 0.f, 1.f} };
    unsigned sunIndices[2][3] = { {0, 1, 3}, {1, 2, 3} };

    Shader sunProg( (path_shaders + "sun.vs").c_str(), (path_shaders + "sun.fs").c_str() );

    unsigned sunVAO = createVAO();
    unsigned sunVBO = createVBO(sizeof(float) * 4*5, sunVertex, GL_STATIC_DRAW);
    unsigned sunEBO = createEBO(sizeof(unsigned) * 2*3, sunIndices, GL_STATIC_DRAW);

    int sizesAtribsSun[2] = { 3, 2 };
    configVAO(sunVAO, sunVBO, sunEBO, sizesAtribsSun, 2);

    sunProg.UseProgram();
    sunProg.setInt("sunTexture",  8);

    // >>> Icosahedron (light source)
    /*
    Icosahedron icos;

    Shader lsProg( (path_shaders + "icosahedron.vs").c_str(), (path_shaders + "icosahedron.fs").c_str() );

    unsigned lsVAO = createVAO();
    unsigned lsVBO = createVBO(sizeof(float) *icos.numVerticesx3, icos.vertices, GL_STATIC_DRAW);
    unsigned int lsEBO = createEBO(sizeof(unsigned)*icos.numIndicesx3, icos.indices, GL_STATIC_DRAW);

    int sizesAtribsLightSource[1] = { 3 };
    configVAO(lsVAO, lsVBO, lsEBO, sizesAtribsLightSource, 1);
    */

    // ----- Other operations

    myGUI gui(window);

    //averager avg;
    //timerSet terrainTime;
    //terrainTime.startTime();

    timer.startTime();
    
    // >>>>> RENDER LOOP <<<<<
    while (!glfwWindowShouldClose(window))
    {
        timer.computeDeltaTime();
        //timer.printTimeData();
        
        processInput(window);

        std::string title = "Simulator (fps: " + std::to_string(timer.getFPS()) + ")";
        glfwSetWindowTitle(window, title.c_str());

        // RENDERING ------------------------
        // ----------------------------------

        glClearColor(skyColor.x, skyColor.y, skyColor.z, skyColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);           // GL_STENCIL_BUFFER_BIT

        // GUI
        gui.implement_NewFrame();
        //GUI_terrainConfig(VAO, VBO, EBO);
        mouseOverGUI = gui.cursorOverGUI();

        // >>> Terrain
        worldChunks.updateVisibleChunks(cam.Position);

        setUniformsTerrain(terrProgram);

        //terrainTime.computeDeltaTime();
        updateTerrain(VAO, VBO, EBO);
        //terrainTime.computeDeltaTime();
        //avg.addValue(terrainTime.getDeltaTime());

        // >>> Water
        setUniformsWater(waterProg);
        glBindVertexArray(waterVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // >>> Axis
        setUniformsAxis(axisProg);
        glBindVertexArray(axisVAO);
        glDrawArrays(GL_LINES, 0, 6);

        // >>> Sun
        setUniformsSun(sunProg, sunLight.direction, 0.2);
        glBindVertexArray(sunVAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sunEBO);
        glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // >>> Icosahedron (light source)
        // setUniformsLightSource(lsProg);
        // glBindVertexArray(lsVAO);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lsEBO);
        // glDrawElements(GL_TRIANGLES, icos.numIndicesx3, GL_UNSIGNED_INT, nullptr);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // GUI
        gui.render();

        // ----------------------------------
        // ----------------------------------

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Render loop End


    // ----- De-allocate all resources

    #ifdef SINGLE_VAO
        glDeleteVertexArrays(1, &VAO);
        for( std::map<BinaryKey, unsigned int>::const_iterator it = VBO.begin();
             it != VBO.end();
             ++it )
        {
            BinaryKey key = it->first;
            glDeleteBuffers     (1, &VBO[key]);
            glDeleteBuffers     (1, &EBO[key]);
        }
    #elif MANY_VAO
        for( std::map<BinaryKey, unsigned int>::const_iterator it = VAO.begin();
             it != VAO.end();
             ++it )
        {
            BinaryKey key = it->first;
            glDeleteVertexArrays(1, &VAO[key]);
            glDeleteBuffers     (1, &VBO[key]);
            glDeleteBuffers     (1, &EBO[key]);
        }
    #endif

    glDeleteProgram(terrProgram.ID);

    glDeleteVertexArrays(1, &axisVAO);
    glDeleteBuffers(1, &axisVBO);
    glDeleteProgram(axisProg.ID);

    glDeleteVertexArrays(1, &waterVAO);
    glDeleteBuffers(1, &waterVBO);
    glDeleteProgram(waterProg.ID);

    glDeleteVertexArrays(1, &sunVAO);
    glDeleteBuffers(1, &sunVBO);
    glDeleteBuffers(1, &sunEBO);
    glDeleteProgram(sunProg.ID);

    //glDeleteVertexArrays(1, &icosVAO);
    //glDeleteBuffers(1, &icosVBO);
    //glDeleteBuffers(1, &icosEBO);
    //glDeleteProgram(icosProg.ID);

    gui.cleanup();
    
    glfwTerminate();

    //std::cout << "Average time: " << avg.getAverage() << std::endl;
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
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cam.ProcessKeyboard(UP, timer.getDeltaTime());
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cam.ProcessKeyboard(DOWN, timer.getDeltaTime());
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

void cleanTerrainBuffers(std::map<BinaryKey, unsigned int> &VAO, std::map<BinaryKey, unsigned int> &VBO, std::map<BinaryKey, unsigned int> &EBO)
{
    for(std::map<BinaryKey, unsigned int>::const_iterator it = VAO.begin();
        it != VAO.end();
        ++it)
    {
        BinaryKey key = it->first;

        glDeleteVertexArrays(1, &VAO[key]);
        glDeleteBuffers     (1, &VBO[key]);
        glDeleteBuffers     (1, &EBO[key]);
    }

    VAO.clear();
    VBO.clear();
    EBO.clear();
}

void GUI_terrainConfig(std::map<BinaryKey, unsigned int> &VAO, std::map<BinaryKey, unsigned int> &VBO, std::map<BinaryKey, unsigned int> &EBO)
{
    // Window
    ImGui::Begin("Noise configuration");
    //ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::Text("Terrain mapping:");

    bool updateTerrain = false;
    if( ImGui::SliderFloat("Max. view distance", &worldChunks.maxViewDist, 1, 500) ) updateTerrain = true;
    if( ImGui::SliderFloat("Chunk size", &worldChunks.chunkSize, 20, 100)          ) updateTerrain = true;
    if( ImGui::SliderInt("VertexPerSide", &worldChunks.vertexPerSide, 5, 50)       ) updateTerrain = true;
    if(updateTerrain)
    {
        worldChunks.updateTerrainParameters(worldChunks.noise, worldChunks.maxViewDist, worldChunks.chunkSize, worldChunks.vertexPerSide);
        cleanTerrainBuffers(VAO, VBO, EBO);
    }

    ImGui::Text("Noise configuration: ");

    const char* noiseTypeString[6] = { "OpenSimplex2", "OpenSimplex2S", "Cellular", "Perlin", "ValueCubic", "Value" };
    int noiseType     = noise.getNoiseType();
    ImGui::Combo      ("Noise type", &noiseType, noiseTypeString, IM_ARRAYSIZE(noiseTypeString));
    int numOctaves    = noise.getNumOctaves();
    ImGui::SliderInt  ("Octaves", &numOctaves, 1, 10);
    float lacunarity  = noise.getLacunarity();
    ImGui::SliderFloat("Lacunarity", &lacunarity, 1.0f, 2.5f);
    float persistance = noise.getPersistance();
    ImGui::SliderFloat("Persistance", &persistance, 0.0f, 1.0f);
    float scale       = noise.getScale();
    ImGui::SliderFloat("Scale", &scale, 0.01f, 1.0f);
    int multiplier    = noise.getMultiplier();
    ImGui::SliderInt  ("Multiplier", &multiplier, 1, 200);
    int curveDegree   = noise.getCurveDegree();
    ImGui::SliderInt  ("Curve degree", &curveDegree, 0, 10);

    ImGui::Text("Map selection: ");

    int seed          = noise.getSeed();
    ImGui::SliderInt("Seed", &seed, 0, 100000);
    int offsetX       = noise.getOffsetX();
    int offsetY       = noise.getOffsetY();
    ImGui::SliderInt("X offset", &offsetX, -500, 500);
    ImGui::SliderInt("Y offset", &offsetY, -500, 500);

    noiseSet newNoise((unsigned)numOctaves, lacunarity, persistance, scale, multiplier, curveDegree, offsetX, offsetY, (FastNoiseLite::NoiseType)noiseType, true, (unsigned)seed);
    if( noise != newNoise)
    {
        noise = newNoise;
        worldChunks.setNoise(noise);
        worldChunks.chunkDict.clear();
        cleanTerrainBuffers(VAO, VBO, EBO);
    }

    ImGui::Text("Water: ");
    ImGui::SliderFloat("Sea level", &seaLevel, -1, 100);

    ImGui::Text("Camera: ");
    ImGui::SliderFloat("Speed", &cam.MovementSpeed, 0, 200);

    //ImGui::ColorEdit3("clear color", (float*)&clear_color);
    //if (ImGui::Button("Button")) counter++;  ImGui::SameLine();  ImGui::Text("counter = %d", counter);
    //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();




/*
    // Types conversion
    noiseSet newNoise((unsigned)numOctaves, lacunarity, persistance, scale, multiplier, curveDegree, offsetX, offsetY, (FastNoiseLite::NoiseType)noiseType, false, (unsigned)seed);

    if( noise != newNoise || dimensionX != terrain.getXside() ||  dimensionY != terrain.getYside() )
    {
        noise = newNoise;
        terrain.computeTerrain(noise, 0, 0, 1, dimensionX, dimensionY);
        newTerrain = true;
    }
*/
}

void setUniformsTerrain(Shader &program)
{
    program.UseProgram();

    // >>> Vertex shader uniforms
    glm::mat4 projection = cam.GetProjectionMatrix();
    program.setMat4("projection", projection);

    glm::mat4 view = cam.GetViewMatrix();
    program.setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    program.setMat4("model", model);

    glm::mat3 normalMatrix = glm::mat3( glm::transpose(glm::inverse(model)) );      // Used when the model matrix applies non-uniform scaling (normals won't be scaled correctly). Otherwise, use glm::vec3(model)
    program.setMat3("normalMatrix", normalMatrix);

    // >>> Fragment shader uniforms
    program.setInt  ("sun.lightType",       sunLight.lightType);
    program.setVec3 ("sun.position",        sunLight.position);
    program.setVec3 ("sun.direction",       sunLight.direction);
    program.setVec3 ("sun.ambient",         sunLight.ambient);
    program.setVec3 ("sun.diffuse",         sunLight.diffuse);
    program.setVec3 ("sun.specular",        sunLight.specular);
    program.setFloat("sun.constant",        sunLight.constant);
    program.setFloat("sun.linear",          sunLight.linear);
    program.setFloat("sun.quadratic",       sunLight.quadratic);
    program.setFloat("sun.cutOff",          sunLight.cutOff);
    program.setFloat("sun.outerCutOff",     sunLight.outerCutOff);

    program.setVec3 ("grass.diffuse",       grass.diffuse);
    program.setVec3 ("grass.specular",      grass.specular);
    program.setFloat("grass.shininess",     grass.shininess);

    program.setVec3 ("rock.diffuse",        rock.diffuse);
    program.setVec3 ("rock.specular",       rock.specular);
    program.setFloat("rock.shininess",      rock.shininess);

    program.setVec3 ("snow.diffuse",        snow.diffuse);
    program.setVec3 ("snow.specular",       snow.specular);
    program.setFloat("snow.shininess",      snow.shininess);

    //program.setVec3 ("sand.diffuse",       sand.diffuse);
    //program.setVec3 ("sand.specular",      sand.specular);
    program.setFloat("sand.shininess",      sand.shininess);

    //program.setVec3 ("plainSand.diffuse",   sand.diffuse);
    //program.setVec3 ("plainSand.specular",  sand.specular);
    program.setFloat("plainSand.shininess", sand.shininess);

    program.setVec4 ("skyColor",            skyColor);
    program.setFloat("fogMaxSquareRadius",  fogMaxR * fogMaxR);
    program.setFloat("fogMinSquareRadius",  fogMinR * fogMinR);

    program.setVec3 ("camPos",              cam.Position);

    // Textures uniforms
    glActiveTexture(GL_TEXTURE0);               // Bind textures on corresponding texture unit
    glBindTexture(GL_TEXTURE_2D, grass.diffuseT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, grass.specularT);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, rock.diffuseT);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, rock.specularT);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, sand.diffuseT);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, sand.specularT);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, plainSand.diffuseT);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, plainSand.specularT);
}

void setUniformsWater(Shader& program)
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

    glm::mat3 normalMatrix = glm::mat3( glm::transpose(glm::inverse(model)) );      // Used when the model matrix applies non-uniform scaling (normal won't be scaled correctly). Otherwise, use glm::vec3(model)
    program.setMat3("normalMatrix", normalMatrix);

    // Fragment shader uniforms
    program.setInt("sun.lightType",  sunLight.lightType);
    program.setVec3("sun.position",  sunLight.position);
    program.setVec3("sun.direction", sunLight.direction);
    program.setVec3("sun.ambient",   sunLight.ambient);
    program.setVec3("sun.diffuse",   sunLight.diffuse);
    program.setVec3("sun.specular",  sunLight.specular);

    program.setVec3("water.diffuse", water.diffuse);
    program.setVec3("water.specular", water.specular);
    program.setFloat("water.shininess", water.shininess);

    program.setVec3("viewPos", cam.Position);
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

void setUniformsSun(Shader& program, glm::vec3 direction, float sunFOV)
{
    program.UseProgram();

    // Vertex shader uniforms
    glm::mat4 projection = cam.GetProjectionMatrix();
    program.setMat4("projection", projection);

    glm::mat4 view = cam.GetViewMatrix();
    program.setMat4("view", view);

    float angleZ   = std::atan(direction.x / direction.y);
    float angleX   = std:: atan(direction.z / std::sqrt(direction.x * direction.x + direction.y * direction.y)) + 3.14159265359/2;
    float distance = worldChunks.maxViewDist * 1.5;
    float scale    = 2 * (std::tan(sunFOV/2) * distance);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate( model, glm::vec3( cam.Position.x + distance * direction.x,
                                              cam.Position.y + distance * direction.y,
                                              cam.Position.z + distance * direction.z) );
    model = glm::rotate   ( model, -angleZ, glm::vec3(0.0f, 0.0f, 1.0f) );
    model = glm::rotate   ( model,  angleX, glm::vec3(1.0f, 0.0f, 0.0f) );
    model = glm::scale    ( model, glm::vec3(scale, scale, scale) );
    program.setMat4("model", model);

    // Textures uniforms
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, sun.diffuseT);
}

void setUniformsLightSource(Shader& program)
{
    program.UseProgram();

    // Vertex shader uniforms
    glm::mat4 projection = cam.GetProjectionMatrix();
    program.setMat4("projection", projection);

    glm::mat4 view = cam.GetViewMatrix();
    program.setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sunLight.direction*1000.f);
    model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(50.0f, 50.0f, 50.0f));
    program.setMat4("model", model);

    // Fragment shader uniforms
    program.setVec4("lightColor", glm::vec4(sunLight.diffuse, 1.f));
}

void updateTerrain_X(std::map<BinaryKey, unsigned int> &VAO, std::map<BinaryKey, unsigned int> &VBO, std::map<BinaryKey, unsigned int> &EBO)
{
    // Delete OGL buffers (VAO, VBO, EBO) not existing in chunks dictionary
    std::vector<BinaryKey> delet;

    for(std::map<BinaryKey, unsigned int>::const_iterator it = VAO.begin();
        it != VAO.end();
        ++it)
    {
        BinaryKey key = it->first;

        if(worldChunks.chunkDict.find(key) == worldChunks.chunkDict.end())
        {
            glDeleteVertexArrays(1, &VAO[key]);
            glDeleteBuffers     (1, &VBO[key]);
            glDeleteBuffers     (1, &EBO[key]);

            delet.push_back(key);
        }
    }

    // Delete fields from all the std::maps, including the dictionary of chunks
    for(size_t i = 0; i < delet.size(); i++)
    {
        worldChunks.chunkDict.erase(delet[i]);
        VAO.erase(delet[i]);
        VBO.erase(delet[i]);
        EBO.erase(delet[i]);
    }

    // Draw elements from the std::maps (VAO, VBO, EBO)
    for(std::map<BinaryKey, terrainGenerator>::const_iterator it = worldChunks.chunkDict.begin();
        it != worldChunks.chunkDict.end();
        it++)
    {
        BinaryKey key = it->first;

        // TODO: Creating new VAO requires (for some unknown reason) specifying "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO)" before subsequent "glDrawElements()". Otherwise, segmentation fault happens.
        // If key doesn't exist, create new field in VAO
        if(VAO.find(key) == VAO.end())
        {
            VAO[key] = createVAO();

            VBO[key] = createVBO( sizeof(float) * worldChunks.getNumVertex() * 8,
                                  worldChunks.chunkDict[key].vertex,
                                  GL_STATIC_DRAW );

            EBO[key] = createEBO(sizeof(unsigned) * worldChunks.getNumIndices(),
                                 worldChunks.chunkDict[key].indices,
                                 GL_STATIC_DRAW );

            int sizesAttribs[3] = {3, 2, 3};
            configVAO( VAO[key], VBO[key], EBO[key], sizesAttribs, 3 );
        }

        //Draw elements from the std::maps
        glBindVertexArray(VAO[key]);    // TODO: Use a single VAO for all terrain chunks, if possible
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[key]);
        glDrawElements(GL_TRIANGLES, worldChunks.getNumIndices(), GL_UNSIGNED_INT, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void updateTerrain(unsigned VAO, std::map<BinaryKey, unsigned int> &VBO, std::map<BinaryKey, unsigned int> &EBO)
{
    glBindVertexArray(VAO);
    int sizesAttribs[3] = {3, 2, 3};

    // Delete OGL buffers (VAO, VBO, EBO) not existing in chunks dictionary
    std::vector<BinaryKey> delet;

    for(std::map<BinaryKey, unsigned int>::const_iterator it = VBO.begin();
        it != VBO.end();
        ++it)
    {
        BinaryKey key = it->first;

        if(worldChunks.chunkDict.find(key) == worldChunks.chunkDict.end())
        {
            //glDeleteVertexArrays(1, &VAO[key]);
            glDeleteBuffers     (1, &VBO[key]);
            glDeleteBuffers     (1, &EBO[key]);

            delet.push_back(key);
        }
    }

    // Delete fields from all the std::maps, including the dictionary of chunks
    for(size_t i = 0; i < delet.size(); i++)
    {
        worldChunks.chunkDict.erase(delet[i]);
        //VAO.erase(delet[i]);
        VBO.erase(delet[i]);
        EBO.erase(delet[i]);
    }

    // Draw elements from the std::maps (VAO, VBO, EBO)
    for(std::map<BinaryKey, terrainGenerator>::const_iterator it = worldChunks.chunkDict.begin();
        it != worldChunks.chunkDict.end();
        it++)
    {
        BinaryKey key = it->first;

        // TODO: Creating new VAO requires (for some unknown reason) specifying "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO)" before subsequent "glDrawElements()". Otherwise, segmentation fault happens.
        // If key doesn't exist, create new field in VAO
        if(VBO.find(key) == VBO.end())
        {
            //VAO[key] = createVAO();//ans2

            VBO[key] = createVBO( sizeof(float) * worldChunks.getNumVertex() * 8,
                                  worldChunks.chunkDict[key].vertex,
                                  GL_STATIC_DRAW );

            EBO[key] = createEBO(sizeof(unsigned) * worldChunks.getNumIndices(),
                                 worldChunks.chunkDict[key].indices,
                                 GL_STATIC_DRAW );

            //int sizesAttribs[3] = {3, 2, 3};
            //configVAO( VAO, VBO[key], EBO[key], sizesAttribs, 3 );
        }

        //Draw elements from the std::maps
        //setUniformsTest(testProg);           // Set uniforms

        configVAO(VAO, VBO[key], EBO[key], sizesAttribs, 3, false);
        //glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[key]);
        glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, nullptr);
        glDrawElements(GL_TRIANGLES, worldChunks.getNumIndices(), GL_UNSIGNED_INT, nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void updateTerrain(std::map<BinaryKey, unsigned int> &VAO, std::map<BinaryKey, unsigned int> &VBO, std::map<BinaryKey, unsigned int> &EBO)
{
    // Delete OGL buffers (VAO, VBO, EBO) not existing in chunks dictionary
    std::vector<BinaryKey> delet;

    for(std::map<BinaryKey, unsigned int>::const_iterator it = VAO.begin();
        it != VAO.end();
        ++it)
    {
        BinaryKey key = it->first;

        if(worldChunks.chunkDict.find(key) == worldChunks.chunkDict.end())
        {
            glDeleteVertexArrays(1, &VAO[key]);
            glDeleteBuffers     (1, &VBO[key]);
            glDeleteBuffers     (1, &EBO[key]);

            delet.push_back(key);
        }
    }

    // Delete fields from all the std::maps, including the dictionary of chunks
    for(size_t i = 0; i < delet.size(); i++)
    {
        worldChunks.chunkDict.erase(delet[i]);
        VAO.erase(delet[i]);
        VBO.erase(delet[i]);
        EBO.erase(delet[i]);
    }

    // Draw elements from the std::maps (VAO, VBO, EBO)
    for(std::map<BinaryKey, terrainGenerator>::const_iterator it = worldChunks.chunkDict.begin();
        it != worldChunks.chunkDict.end();
        it++)
    {
        BinaryKey key = it->first;

        // TODO: Creating new VAO requires (for some unknown reason) specifying "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO)" before subsequent "glDrawElements()". Otherwise, segmentation fault happens.
        // If key doesn't exist, create new field in VAO
        if(VAO.find(key) == VAO.end())
        {
            VAO[key] = createVAO();

            VBO[key] = createVBO( sizeof(float) * worldChunks.getNumVertex() * 8,
                                  worldChunks.chunkDict[key].vertex,
                                  GL_STATIC_DRAW );

            EBO[key] = createEBO(sizeof(unsigned) * worldChunks.getNumIndices(),
                                 worldChunks.chunkDict[key].indices,
                                 GL_STATIC_DRAW );

            int sizesAttribs[3] = {3, 2, 3};
            configVAO( VAO[key], VBO[key], EBO[key], sizesAttribs, 3 );
        }

        //Draw elements from the std::maps
        glBindVertexArray(VAO[key]);    // TODO: Use a single VAO for all terrain chunks, if possible
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[key]);
        glDrawElements(GL_TRIANGLES, worldChunks.getNumIndices(), GL_UNSIGNED_INT, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void setUniformsTest(Shader &program)
{
    program.UseProgram();

    // Vertex shader uniforms
    glm::mat4 projection = cam.GetProjectionMatrix();
    program.setMat4("projection", projection);

    glm::mat4 view = cam.GetViewMatrix();
    program.setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate( model, glm::vec3( 0, 0, 0 ) );
    model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale    ( model, glm::vec3(50, 50, 50) );
    program.setMat4("model", model);

    // Textures uniforms
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, sun.diffuseT);
}




























