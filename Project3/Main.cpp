
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include "cyVector.h"
#include "cyGL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include "RenderableObject.h"
#include "PointLight.h"
#include "Shader.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

static std::string ExecutableDirectory;

static GLuint ShaderProgram;
static GLint mvpUniformLocation;

//Camera distance changing.
#define CAMERA_MINIMUM_DISTANCE 12.0f
#define CAMERA_MAXIMUM_DISTANCE 200.0f
static float cameraDistance = 25.0f;
static bool draggingMouseWithRMB = false;
static double lastRMBPositionY;
static float distancePerPixel = 0.05f;

//Camera angle changing
static float cameraAngleX = 0.0f;
static float cameraAngleY = 0.0f;
static bool draggingMouseWithLMB = false;
static float angleChangePerPixel = 0.005f;
static double lastLMBPositionX;
static double lastLMBPositionY;


static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        draggingMouseWithRMB = (action == GLFW_PRESS);
        if (draggingMouseWithRMB)
        {
            lastRMBPositionY = ypos;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        draggingMouseWithLMB = (action == GLFW_PRESS);
        if (draggingMouseWithLMB)
        {
            lastLMBPositionX = xpos;
            lastLMBPositionY = ypos;
        }
    }
}

static void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (draggingMouseWithLMB)
    {
        double xDifference = xpos - lastLMBPositionX;
        lastLMBPositionX = xpos;
        cameraAngleY -= xDifference * angleChangePerPixel;


        double yDifference = ypos - lastLMBPositionY;
        lastLMBPositionY = ypos;
        cameraAngleX -= yDifference * angleChangePerPixel;
    }

    if (draggingMouseWithRMB)
    {
        double yDifference = ypos - lastRMBPositionY;
        lastRMBPositionY = ypos;
        cameraDistance += yDifference * distancePerPixel;
        if (cameraDistance < CAMERA_MINIMUM_DISTANCE)
        {
            cameraDistance = CAMERA_MINIMUM_DISTANCE;
        }
        else if (cameraDistance > CAMERA_MAXIMUM_DISTANCE)
        {
            cameraDistance = CAMERA_MAXIMUM_DISTANCE;
        }
    }
}

static void errorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Requires a single argument for the obj file location\n");
        return 0;
    }
    std::string exePath(argv[0]);
    ExecutableDirectory = std::string(exePath.substr(0, exePath.find_last_of('\\')));

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
        return -1;

    GLFWwindow * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mousePosCallback);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return -1;
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));


    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glClearColor(0, 0, 0, 1);

    std::string fragShaderPath(ExecutableDirectory);
    fragShaderPath.append("\\shader.frag");

    std::string vertexShaderPath(ExecutableDirectory);
    vertexShaderPath.append("\\shader.vert");

    PointLight light;
    light.LightPosition = cyVec4f(0.0, 0.0, 20.0, 1.0);
    light.LightIntensity = 1.0f;

    Material material;

    material.AmbientDiffuseColor = cyVec4f(0, 0.5, 0, 1.0);
    float ambientIntensity = 0.1f;
    material.SpecularShininess = 10;
    material.SpecularColor = cyVec4f(1.0, 1.0, 1.0, 1.0);

    RenderableObject renderable(argv[1], &material);
    renderable.CenterOnBoundingBox = true;

    Shader shader(vertexShaderPath, fragShaderPath);

    float fov = 1.570796326f; //pi/2 radians
    float aspectRatio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    cyMatrix4f perspectiveTransform = cyMatrix4f::Perspective(fov, aspectRatio, 1, 1000.0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cyMatrix4f cameraTransform = cyMatrix4f::Translation(cyVec3f(0,0,-cameraDistance)) * cyMatrix4f::RotationY(-cameraAngleY) * cyMatrix4f::RotationX(-cameraAngleX);
        cyMatrix4f cameraPositionTransform = cyMatrix4f::Translation(cyVec3f(0, 0, cameraDistance)) * cyMatrix4f::RotationY(cameraAngleY) * cyMatrix4f::RotationX(cameraAngleX);
        cyVec4f cameraPosition = cameraPositionTransform * cyVec4f(0.0, 0.0, 0.0, 1.0);
        shader.Draw(&renderable, &light, cameraTransform, perspectiveTransform, cameraPosition, ambientIntensity);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}