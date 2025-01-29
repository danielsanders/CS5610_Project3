
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include "cyVector.h"
#include "cyGL.h"
#include <stdio.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <cmath>

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
static float distancePerPixel = 0.05;

//Camera angle changing
static float cameraAngleX = 0.0f;
static float cameraAngleY = 0.0f;
static bool draggingMouseWithLMB = false;
static float angleChangePerPixel = 0.005;
static double lastLMBPositionX;
static double lastLMBPositionY;

static bool CompileShadersAndGetUniformLocations()
{
    bool success = false;
    char infoLog[512];
    std::string vertexShaderPath(ExecutableDirectory);
    vertexShaderPath.append("\\shader.vert");
    std::ifstream vertInput(vertexShaderPath);
    std::stringstream vertexStringBuffer;
    vertexStringBuffer << vertInput.rdbuf();
    std::string vertexShaderString = vertexStringBuffer.str();
    GLchar const* vertexShaderSource = vertexShaderString.c_str();
    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int vertexSuccess;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexSuccess);
    if (!vertexSuccess)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        fprintf(stderr, "Failed to compile vertex shader. Error: %s\n", infoLog);
    }

    std::string fragShaderPath(ExecutableDirectory);
    fragShaderPath.append("\\shader.frag");
    std::ifstream fragInput(fragShaderPath);
    std::stringstream fragStringBuffer;
    fragStringBuffer << fragInput.rdbuf();
    std::string fragShaderString = fragStringBuffer.str();
    GLchar const* fragShaderSource = fragShaderString.c_str();
    GLuint fragShader;
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragShaderSource, NULL);
    glCompileShader(fragShader);
    int fragSuccess;
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &fragSuccess);
    if (!fragSuccess)
    {
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        fprintf(stderr, "Failed to compile fragment shader. Error: %s\n", infoLog);
    }

    if (vertexSuccess && fragSuccess)
    {
        GLuint newShaderProgram;
        newShaderProgram = glCreateProgram();
        glAttachShader(newShaderProgram, vertexShader);
        glAttachShader(newShaderProgram, fragShader);
        glLinkProgram(newShaderProgram);
        int linkSuccess;
        glGetProgramiv(newShaderProgram, GL_LINK_STATUS, &linkSuccess);
        if (!linkSuccess)
        {
            glGetProgramInfoLog(newShaderProgram, 512, NULL, infoLog);
            fprintf(stderr, "Failed to link shaders. Error: %s\n", infoLog);
        }
        else
        {
            GLint mvpLoc = glGetUniformLocation(newShaderProgram, "mvp");
            if (mvpLoc == -1)
            {
                fprintf(stderr, "Could not get mvp uniform location.\n");
            }
            else
            {
                mvpUniformLocation = mvpLoc;
                ShaderProgram = newShaderProgram;
                success = true;
            }
        }
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);

    return success;
}

static bool RecompileShadersAndGetUniformLocations()
{
    GLuint oldProgram = ShaderProgram;
    bool success = CompileShadersAndGetUniformLocations();
    if (success)
    {
        glDeleteProgram(oldProgram);
    }
    return success;
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_F6 && action == GLFW_RELEASE)
    {
        bool success = RecompileShadersAndGetUniformLocations();
        if (!success)
        {
            fprintf(stderr, "Failed to recompile and use new shaders. Keeping previous shaders.\n");
        }
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

    cy::TriMesh mesh;
    bool loadObjSuccess = mesh.LoadFromFileObj(argv[1]);
    if (!loadObjSuccess)
    {
        fprintf(stderr, "Could not load obj file\n");
        return -1;
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    bool success = CompileShadersAndGetUniformLocations();

    if (!success)
    {
        fprintf(stderr, "Failed to set up initial shaders. Exiting.\n");
        return -1;
    }

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    int numVertices = mesh.NV();
    float* verticesBuffer = new float[numVertices * 3];
    for (int i = 0; i < numVertices; i++)
    {
        cy::Vec3f vertex = mesh.V(i);
        verticesBuffer[i * 3] = vertex.x;
        verticesBuffer[i * 3 + 1] = vertex.y;
        verticesBuffer[i * 3 + 2] = vertex.z;
    }
    glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(float), verticesBuffer, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    float fov = 1.570796326; //pi/2 radians
    float aspectRatio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    cyMatrix4f perspectiveTransform = cyMatrix4f::Perspective(fov, aspectRatio, 1, 1000.0);

    mesh.ComputeBoundingBox();
    cyVec3f center = (mesh.GetBoundMax() + mesh.GetBoundMin()) / 2;

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        cyMatrix4f modelTransform = cyMatrix4f::Translation(-1 * center);

        cyMatrix4f cameraTransform = cyMatrix4f::Translation(cyVec3f(0,0,-cameraDistance)) * cyMatrix4f::RotationY(-cameraAngleY)* cyMatrix4f::RotationX(-cameraAngleX);

        cyMatrix4f mvpMatrix = perspectiveTransform * cameraTransform * modelTransform;

        glBindVertexArray(vao);
        glUseProgram(ShaderProgram);

        float mvpMatrixValues[4][4];
        mvpMatrix.Get(&mvpMatrixValues[0][0]);

        glUniformMatrix4fv(mvpUniformLocation, 1, GL_FALSE, &mvpMatrixValues[0][0]);

        glDrawArrays(GL_POINTS, 0, numVertices);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    delete[] verticesBuffer;

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}