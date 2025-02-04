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
#include "Camera.h"

class Shader
{
public:
	Shader(std::string vertexShaderFilename, std::string fragShaderFilename);

	void Draw(RenderableObject* object, PointLight* light, Camera* camera, cyMatrix4f projectionTransform, float ambientLightIntensity);

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

