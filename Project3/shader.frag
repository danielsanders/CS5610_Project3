#version 330 core

in vec3 SurfaceNormal;
in vec4 ViewSpacePosition;

out vec4 FragColor;

void main()
{
	FragColor = vec4(SurfaceNormal.x, SurfaceNormal.y, SurfaceNormal.z, 1.0);
}