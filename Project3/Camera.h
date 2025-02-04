#pragma once
#include <cyMatrix.h>
class Camera
{
	cyVec3f Position;
	cyVec3f RotationAngles;

	cyMatrix4f GetCameraTransform();
};

