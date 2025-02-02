#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;

out vec3 SurfaceNormal;
out vec4 ViewSpacePosition;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat3 mvn;

void main()
{
	gl_Position = mvp * vec4(aPos,1);
	SurfaceNormal = normalize(mvn * aNormal);
	ViewSpacePosition = mv * vec4(aPos,1);
}