#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>

#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <random>
#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::mat4 lightRotation;
glm::vec3 lightPosition;
GLfloat lightAngle;
GLint is_positional;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint lightPositionLoc;
GLint CameraPositionLoc;
GLint lightTypeLoc;
GLint nearPlaneLoc;
GLint fogDensityLoc;
GLint farPlaneLoc;

// Position:(0.618196,-0.123672,0.890926)
gps::Camera myCamera(
    glm::vec3(0.0f, -0.2f, 0.89f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));


GLfloat cameraSpeed = 0.1f;
GLfloat angle;
GLboolean pressedKeys[1024];

// models
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D cactus;
gps::Model3D tumbleweed;
gps::Model3D windmill;
gps::Model3D windmillhead;
gps::Model3D cat;
gps::Model3D cottage;
gps::Model3D stone;
gps::Model3D skull;

// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;
gps::Shader lightShader;
gps::Shader skyBoxShader;

// shadows parameters
GLuint shadowMapFBO;
GLuint depthMapTexture;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

//fog 
GLfloat fogDensity = 0.0f;

//mouse callback 
float lastX = 400, lastY = 300;
float pitch = 0, yaw = -90.0f;
bool firstMouse = true;
float angleY = 0.0f;

// delta time
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//skybox
gps::SkyBox mySkyBox;

float getRandomFloat(float min, float max) {
    std::random_device rd;   
    std::mt19937 e2(rd());

    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(e2);
}

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // Adjust sensitivity as needed
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Ensure that the pitch is within the range of [-89, 89] degrees
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
}

void processMovement() {
    GLfloat cameraSpeed = 2.5 * deltaTime;
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 3.0f;
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 3.0f;
    }

    if (pressedKeys[GLFW_KEY_J]) {
        lightAngle -= 1.0f;
    }

    if (pressedKeys[GLFW_KEY_L]) {
        lightAngle += 1.0f;
    }
    
    if (pressedKeys[GLFW_KEY_1])
    {
         glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (pressedKeys[GLFW_KEY_2]){
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (pressedKeys[GLFW_KEY_3]){
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    if (pressedKeys[GLFW_KEY_O]) {
        myBasicShader.useShaderProgram();
        lightTypeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightType");
        glUniform1i(lightTypeLoc, 0);
        is_positional = 0;
    }
    if (pressedKeys[GLFW_KEY_P]) {
        myBasicShader.useShaderProgram();
        lightTypeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightType");
        glUniform1i(lightTypeLoc, 1);
        is_positional = 1;
    }
    if (pressedKeys[GLFW_KEY_M]) {
        fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
        fogDensity = (fogDensity < 1.0f) ? fogDensity + 0.01f : 1.0f;
        glUniform1f(fogDensityLoc, fogDensity);
    }
    if (pressedKeys[GLFW_KEY_N]) {
        fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
        
        fogDensity = (fogDensity > 0.0f)? fogDensity - 0.01f: 0.0f;
        glUniform1f(fogDensityLoc, fogDensity);
    }
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "Deserted home");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(myWindow.getWindow());
    glfwSwapInterval(1);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

    glEnable(GL_FRAMEBUFFER_SRGB);//
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

    glfwMakeContextCurrent(myWindow.getWindow());

    glfwSwapInterval(1);
}

void initModels() {
    ground.LoadModel("models/ground/ground.obj");
    lightCube.LoadModel("models/cube/cube.obj");
    cactus.LoadModel("models/cactus/10436_Cactus_v1_max2010_it2.obj");
    tumbleweed.LoadModel("models/tumbleweed/tumble.obj");
    windmill.LoadModel("models/windmill/windmill.obj");
    windmillhead.LoadModel("models/windmill/windmill_headobj.obj");
    cat.LoadModel("models/cat/cat.obj");
    cottage.LoadModel("models/cottage/cottage.obj");
    stone.LoadModel("models/rock/rock.obj");
    skull.LoadModel("models/bones/skull.obj");
}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    myBasicShader.useShaderProgram();

    lightShader.loadShader(
        "shaders/lightCube.vert",
        "shaders/lightCube.frag");
    lightShader.useShaderProgram();

    depthMapShader.loadShader(
        "shaders/shadow.vert",
        "shaders/shadow.frag");
    depthMapShader.useShaderProgram();

    skyBoxShader.loadShader(
        "shaders/skyboxShader.vert",
        "shaders/skyboxShader.frag");
    skyBoxShader.useShaderProgram();

}

void initSkyBox() {
	std::vector<const GLchar*> faces;

	faces.push_back("skybox/px.png");
	faces.push_back("skybox/nx.png");
	faces.push_back("skybox/py.png");
	faces.push_back("skybox/ny.png");
	faces.push_back("skybox/pz.png");
	faces.push_back("skybox/nz.png");
	mySkyBox.Load(faces);

}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Create and send view matrix 
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    //Create and send normal matrix 
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// Create and send projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

    // Create and send light direction  
    lightDir = glm::vec3(0.0f, 1.5f, 1.5f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));;
        
    // Create and send light color  
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    //Create and send light position Position:(-0.20185,-0.256673,-1.66408)
    lightPosition = glm::vec3(-0.2f, -0.25f, -1.3f);
    lightPositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPosition");
    glUniform3fv(lightPositionLoc, 1, glm::value_ptr(lightPosition));
    
    fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    glUniform1f(fogDensityLoc, fogDensity);
    
    //Create and send light position 
    lightTypeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightType");
    glUniform1i(lightTypeLoc, 0);
}

void initFBO() {

    glGenFramebuffers(1, &shadowMapFBO);

    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

glm::mat4 computeLightSpaceTrMatrixOrth() {
    
    glm::mat4 lightView = glm::lookAt(glm::vec3(lightRotation * glm::vec4(lightDir, 1.0f)), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const GLfloat near_plane = 0.1f, far_plane =10.0f;

    float orthoSize = 3.0f; 
    glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near_plane, far_plane);
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    return lightSpaceMatrix;
}

glm::mat4 computeLightSpaceTrMatrixPersp() {
    glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const GLfloat near_plane = 0.1f, far_plane = 10.0f;
    GLfloat aspect = static_cast<GLfloat>(SHADOW_WIDTH) / static_cast<GLfloat>(SHADOW_HEIGHT);

    
    GLfloat fov = 2.0f * glm::atan(glm::tan(glm::radians(45.0f) * 0.5f) / aspect);

    glm::mat4 lightProjection = glm::perspective(fov, aspect, near_plane, far_plane);

    glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    return lightSpaceMatrix;
}

void drawCat(gps::Shader shader,bool depthPass) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.2488f, -0.47f, -1.72f));
    model = glm::scale(model, glm::vec3(0.004f));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    cat.Draw(shader);
}

void drawCactus(gps::Shader shader, bool depthPass, glm::vec3 position) {
    model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(0.0009f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    cactus.Draw(shader);
}

void drawGround(gps::Shader shader, bool depthPass) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -1.0f));
    model = glm::scale(model, glm::vec3(0.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    ground.Draw(shader);
}

void drawTumbleweed(gps::Shader shader, bool depthPass,float startx, float endx) {
    
    //X Coordinate
    GLfloat animationSpeed = 0.25 * deltaTime;
    static float currentTumblex = startx;
    currentTumblex = (currentTumblex > endx) ? startx : currentTumblex + animationSpeed;
   
    //Y Coordinate
    GLfloat frequency = 5.0f;
    float outputMin = -0.46f;  
    float outputMax = -0.27f;
    float sineResult = sin(frequency * currentTumblex);
    static float currentTumbley = outputMin;
    currentTumbley = (sineResult - (-1.0f)) / (1.0f - (-1.0f)) * (outputMax - outputMin) + outputMin;

    //Z Coordinate
    static float zCoord = 0.0f;
    if (startx == currentTumblex) {
        zCoord = getRandomFloat(-1.0f, 0.2f);
    }

    model = glm::translate(glm::mat4(1.0f), glm::vec3(currentTumblex, currentTumbley, zCoord));
    model = glm::scale(model, glm::vec3(0.0005f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    tumbleweed.Draw(shader);
}

void drawWindmill(gps::Shader shader, bool depthPass) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.767222f, -0.5f, -1.51006f));
    model = glm::scale(model, glm::vec3(0.1f));
    //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    windmill.Draw(shader);

    //animate windmill
    GLfloat animationSpeed = 20 * deltaTime;
    static float rotation = 0.0f;
    rotation += animationSpeed;
    model = glm::translate(model, glm::vec3(0.0f, 6.93f, 1.0f));
    model=glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    windmillhead.Draw(shader);
}

void drawCottage(gps::Shader shader, bool depthPass) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.4f, -0.5f, -1.5f));
    model = glm::scale(model, glm::vec3(0.025f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    cottage.Draw(shader);
}

void drawRock(gps::Shader shader, bool depthPass) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.1f, -0.5f, 0.17f));
    model = glm::scale(model, glm::vec3(0.07f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    stone.Draw(shader);
}

void drawSkull(gps::Shader shader, bool depthPass) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.51f, 0.0f));
    model = glm::scale(model, glm::vec3(0.003f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    skull.Draw(shader);
}



void drawObjects(gps::Shader shader, bool depthPass) {

    shader.useShaderProgram();

    drawCactus(shader,depthPass, glm::vec3(0.98f, -0.51f, -1.662f));
    drawCactus(shader, depthPass, glm::vec3(-0.44f, -0.51f, -0.32f));
    drawCactus(shader, depthPass, glm::vec3(-1.38f, -0.51f, -0.99f));
    drawCactus(shader, depthPass, glm::vec3(1.23f, -0.51f, -0.36f));
    drawCat(shader, depthPass);
    drawGround(shader, depthPass);
    drawTumbleweed(shader, depthPass,-2.0f,2.0f);
    drawWindmill(shader, depthPass);
    drawCottage(shader, depthPass);
    drawRock(shader, depthPass);
    drawSkull(shader, depthPass);
     
    skyBoxShader.useShaderProgram();
    if (!depthPass) {
        mySkyBox.Draw(skyBoxShader, view, projection);
    }
}



void renderScene() {
    glm::mat4 lightSpaceTrMatrix = (is_positional) ? computeLightSpaceTrMatrixPersp() : computeLightSpaceTrMatrixOrth();

    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceTrMatrix));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    drawObjects(depthMapShader, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    //bind the shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
       GL_FALSE,
        glm::value_ptr(lightSpaceTrMatrix));

    drawObjects(myBasicShader, false);

    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    model = lightRotation;
    model = glm::translate(model, 1.0f * lightDir);
    model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    lightCube.Draw(myBasicShader);

}
void cleanup() {
    myWindow.Delete();

    glDeleteFramebuffers(1, &shadowMapFBO);
    glDeleteTextures(1, &depthMapTexture);
    glfwTerminate();
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    initSkyBox();
    initFBO();
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        std::cout <<"Position:(" << myCamera.getCameraPosition().x<<","<< myCamera.getCameraPosition().y << "," << myCamera.getCameraPosition().z << ")\n";

        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
