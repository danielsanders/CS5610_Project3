#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "cyMatrix.h"


class RenderableObject
{
public:
	RenderableObject(char* objFilename, std::string vertexShaderFilename, std::string fragShaderFilename);

	void Draw(cyMatrix4f cameraTransform, cyMatrix4f perspectiveTransform);

	cyVec3f Position;
	cyVec3f Scale;
	cyVec3f RotationAngles;

	bool CenterOnBoundingBox;

private:

	void CalculateModelTransform();

	int InitializeFromObjFile(char* filename);
	int CompileShaders(std::string vertexShaderFilename, std::string fragShaderFilename);

	GLuint VAO;
	GLuint IndexBuffer;
	int IndexBufferCount;
	GLuint VertexPosElementBufferObject;
	GLuint VertexNormalElementBufferObject;
	GLuint VertexUVElementBufferObject;

	GLuint ShaderProgram;
	GLint MVPUniformLocation;
	GLint MVUniformLocation;
	GLint MVNUniformLocation;

	cyMatrix4f ModelTransform;
	cyVec3f BoundingBoxCenter;
	


};

