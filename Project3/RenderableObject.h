#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "cyMatrix.h"
#include "Material.h"


class RenderableObject
{
public:
	RenderableObject(char* objFilename, Material* material);

	void Draw();

	cyVec3f Position;
	cyVec3f Scale;
	cyVec3f RotationAngles;
	bool CenterOnBoundingBox;

	Material* ObjectMaterial;

	cyMatrix4f CalculateModelTransform();

private:

	int InitializeFromObjFile(char* filename);
	int CompileShaders(std::string vertexShaderFilename, std::string fragShaderFilename);

	GLuint VAO;
	GLuint IndexBuffer;
	int IndexBufferCount;
	GLuint VertexPosElementBufferObject;
	GLuint VertexNormalElementBufferObject;
	GLuint VertexUVElementBufferObject;

	cyVec3f BoundingBoxCenter;
	
};

