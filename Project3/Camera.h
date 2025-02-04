#pragma once
#include <cyMatrix.h>
class Camera
{
public:
	cyVec4f Position;
	cyVec4f Forward;
	cyVec4f Up;

	cyMatrix4f GetCameraTransform();
};

