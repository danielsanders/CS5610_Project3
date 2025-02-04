#pragma once

#include "cyVector.h"
#include "cyMatrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "PointLight.h"
#include "RenderableObject.h"

class Shader
{
public:
	Shader(std::string vertexShaderFilename, std::string fragShaderFilename);

	void Draw(RenderableObject* object, PointLight* light, cyMatrix4f cameraTransform, cyMatrix4f projectionTransform, cyVec4f cameraPosition, float ambientLightIntensity);

private:

	GLuint ShaderProgram;
	GLint MVPUniformLocation;
	GLint MVUniformLocation;
	GLint MVNUniformLocation; 

	GLint DiffuseAmbientColorLocation;
	GLint SpecularColorLocation;
	GLint SpecularShininessLocation;

	GLint AmbientLightIntensityLocation;
	GLint LightIntensityLocation;
	GLint LightPositionLocation;
	GLint CameraPositionLocation;

	int CompileShaders(std::string vertexShaderFilename, std::string fragShaderFilename);
};

