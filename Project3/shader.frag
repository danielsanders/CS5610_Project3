#version 330 core

in vec3 SurfaceNormal;
in vec4 ViewSpacePosition;

uniform vec4 DiffuseAmbientColor;
uniform vec4 SpecularColor;
uniform float SpecularShininess;

uniform float AmbientLightIntensity;
uniform float LightIntensity;
uniform vec3 LightPosition;
uniform vec3 CameraPosition;

out vec4 FragColor;

void main()
{
	vec3 normalizedNormal = normalize(SurfaceNormal);
	vec3 fragPosition = ViewSpacePosition.xyz;
	vec3 lightDirection = normalize(LightPosition - fragPosition);
	vec3 viewDirection = normalize(CameraPosition - fragPosition);
	vec3 halfVector = normalize(lightDirection + viewDirection);
	FragColor = LightIntensity * 
		(
			max(0,dot(lightDirection, normalizedNormal)) * DiffuseAmbientColor + 
			pow(max(0,dot(halfVector, normalizedNormal)), SpecularShininess) * SpecularColor
		) + AmbientLightIntensity * DiffuseAmbientColor;
}