#include "RenderableObject.h"

#include "cyTriMesh.h"
#include <map>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>

struct ObjFileIndexData
{
	int BuffersIndex;
	int VertexPositionIndex;
	int NormalPositionIndex;
	int UVPositionIndex;
};

RenderableObject::RenderableObject(char* objFilename, Material * material)
{
	Position = cyVec3f(0, 0, 0);
	Scale = cyVec3f(1, 1, 1);
	RotationAngles = cyVec3f(0, 0, 0);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	ObjectMaterial = material;

	InitializeFromObjFile(objFilename);

}

cyMatrix4f RenderableObject::CalculateModelTransform()
{
	cyMatrix4f modelTransform = cyMatrix4f::Identity();
	if (CenterOnBoundingBox)
	{
		modelTransform = cyMatrix4f::Translation(-BoundingBoxCenter) * modelTransform;
	}
	modelTransform = cyMatrix4f::Translation(Position) * cyMatrix4f::Scale(Scale) *
		cyMatrix4f::RotationX(RotationAngles.x) * cyMatrix4f::RotationY(RotationAngles.y) * 
		cyMatrix4f::RotationZ(RotationAngles.z) * modelTransform;
	return modelTransform;
}

void RenderableObject::Draw()
{
	glBindVertexArray(VAO);

	glDrawElements(GL_TRIANGLES, IndexBufferCount, GL_UNSIGNED_INT, 0);

}

int RenderableObject::InitializeFromObjFile(char* filename)
{
	cyTriMesh mesh;
	bool loadObjSuccess = mesh.LoadFromFileObj(filename);
	if (!loadObjSuccess)
	{
		fprintf(stderr, "Could not load obj file\n");
		return -1;
	}

	mesh.ComputeBoundingBox();
	BoundingBoxCenter = mesh.GetBoundMin() + (mesh.GetBoundMax() - mesh.GetBoundMin()) / 2;

	std::vector<int> elementBufferVector;
	std::vector<ObjFileIndexData> objIndices;

	std::map<int, std::vector<ObjFileIndexData>> indicesForObjPositionIndex;
	
	for (unsigned int i = 0; i < mesh.NF(); i++)
	{
		cy::TriMesh::TriFace face = mesh.F(i);
		cy::TriMesh::TriFace faceNormal = mesh.FN(i);
		cy::TriMesh::TriFace faceUV = mesh.FT(i);
		
		for (int j = 0; j < 3; j++)
		{
			int vertexObjIndex = face.v[j];
			int normalIndex = faceNormal.v[j];
			int uvIndex = faceUV.v[j];
			std::vector<ObjFileIndexData> indicesForVertex = indicesForObjPositionIndex[vertexObjIndex];
			bool found = false;
			if (indicesForVertex.size() > 0)
			{
				for (ObjFileIndexData indexData : indicesForVertex)
				{
					if (indexData.NormalPositionIndex == normalIndex && indexData.UVPositionIndex == uvIndex)
					{
						found = true;
						elementBufferVector.push_back(indexData.BuffersIndex);
						break;
					}
				}
			}
			if (!found)
			{
				ObjFileIndexData indexData;
				indexData.BuffersIndex = objIndices.size();
				indexData.NormalPositionIndex = normalIndex;
				indexData.UVPositionIndex = uvIndex;
				indexData.VertexPositionIndex = vertexObjIndex;
				indicesForObjPositionIndex[vertexObjIndex].push_back(indexData);
				objIndices.push_back(indexData);
				elementBufferVector.push_back(indexData.BuffersIndex);
			}

		}
	}

	int bufferSize = objIndices.size() * 3 * sizeof(float);
	glGenBuffers(1, &VertexPosElementBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, VertexPosElementBufferObject);
	float* vertexPositions = new float[bufferSize];
	for (int i = 0; i < objIndices.size(); i++)
	{
		cyVec3f vertexPosiion = mesh.V(objIndices[i].VertexPositionIndex);
		vertexPositions[i*3] = vertexPosiion.x;
		vertexPositions[i*3+1] = vertexPosiion.y;
		vertexPositions[i*3+2] = vertexPosiion.z;
	}
	glBufferData(GL_ARRAY_BUFFER, bufferSize, vertexPositions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	delete[] vertexPositions;

	glGenBuffers(1, &VertexNormalElementBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, VertexNormalElementBufferObject);
	float* normals = new float[bufferSize];
	for (int i = 0; i < objIndices.size(); i++)
	{
		cyVec3f normal = mesh.VN(objIndices[i].NormalPositionIndex);
		normals[i * 3] = normal.x;
		normals[i * 3 + 1] = normal.y;
		normals[i * 3 + 2] = normal.z;
	}
	glBufferData(GL_ARRAY_BUFFER, bufferSize, normals, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	delete[] normals;


	glGenBuffers(1, &IndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	IndexBufferCount = elementBufferVector.size();
	float indexBufferSize = elementBufferVector.size() * sizeof(int);
	int* elementsBuffer = new int[indexBufferSize];
	for (int i = 0; i < elementBufferVector.size(); i++)
	{
		elementsBuffer[i] = elementBufferVector[i];
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, elementsBuffer, GL_STATIC_DRAW);
	delete[] elementsBuffer;

	return 0;
}
