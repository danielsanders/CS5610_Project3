#include "Camera.h"

cyMatrix4f Camera::GetCameraTransform()
{
    return cyMatrix4f::View(Position.XYZ(), (Position + Forward).XYZ(), Up.XYZ());
}
