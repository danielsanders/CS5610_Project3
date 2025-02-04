#include "Shader.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>

Shader::Shader(std::string vertexShaderFilename, std::string fragShaderFilename)
{
	CompileShaders(vertexShaderFilename, fragShaderFilename);
}

void Shader::Draw(RenderableObject* object, PointLight* light, cyMatrix4f cameraTransform, cyMatrix4f projectionTransform, cyVec4f cameraPosition, float ambientLightIntensity)
{
	cyMatrix4f modelTransform = object->CalculateModelTransform();

	glUseProgram(ShaderProgram);

	cyMatrix4f mvp = projectionTransform * cameraTransform * modelTransform;
	float mvpMatrixValues[4][4];
	mvp.Get(&mvpMatrixValues[0][0]);
	glUniformMatrix4fv(MVPUniformLocation, 1, GL_FALSE, &mvpMatrixValues[0][0]);
	
	cyMatrix4f mv = cameraTransform * modelTransform;
	float mvMatrixValues[4][4];
	mv.Get(&mvMatrixValues[0][0]);
	glUniformMatrix4fv(MVUniformLocation, 1, GL_FALSE, &mvMatrixValues[0][0]);

	cyMatrix3f mvn = mv.GetSubMatrix3();
	mvn.Invert();
	mvn.Transpose();
	float mvnMatrixValues[3][3];
	mvn.Get(&mvnMatrixValues[0][0]);
	glUniformMatrix3fv(MVNUniformLocation, 1, GL_FALSE, &mvnMatrixValues[0][0]);
	
	glUniform4fv(DiffuseAmbientColorLocation, 1, &object->ObjectMaterial->AmbientDiffuseColor[0]);
	glUniform4fv(SpecularColorLocation, 1, &object->ObjectMaterial->SpecularColor[0]);
	glUniform1f(SpecularShininessLocation, object->ObjectMaterial->SpecularShininess);

	glUniform1f(AmbientLightIntensityLocation, ambientLightIntensity);
	glUniform1f(LightIntensityLocation, light->LightIntensity);
	cyVec4f lightPositionInViewSpace = cameraTransform * light->LightPosition;
	glUniform3fv(LightPositionLocation, 1, &lightPositionInViewSpace[0]);
	glUniform3fv(CameraPositionLocation, 1, &cameraPosition[0]);
	object->Draw();
}

int Shader::CompileShaders(std::string vertexShaderFilename, std::string fragShaderFilename)
{
	char infoLog[512];
	std::ifstream vertInput(vertexShaderFilename);
	std::stringstream vertexStringBuffer;
	vertexStringBuffer << vertInput.rdbuf();
	std::string vertexShaderString = vertexStringBuffer.str();
	GLchar const* vertexShaderSource = vertexShaderString.c_str();
	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	int vertexSuccess;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexSuccess);
	if (!vertexSuccess)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		fprintf(stderr, "Failed to compile vertex shader. Error: %s\n", infoLog);
	}

	std::ifstream fragInput(fragShaderFilename);
	std::stringstream fragStringBuffer;
	fragStringBuffer << fragInput.rdbuf();
	std::string fragShaderString = fragStringBuffer.str();
	GLchar const* fragShaderSource = fragShaderString.c_str();
	GLuint fragShader;
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragShaderSource, NULL);
	glCompileShader(fragShader);
	int fragSuccess;
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &fragSuccess);
	if (!fragSuccess)
	{
		glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
		fprintf(stderr, "Failed to compile fragment shader. Error: %s\n", infoLog);
	}

	if (vertexSuccess && fragSuccess)
	{
		GLuint newShaderProgram;
		newShaderProgram = glCreateProgram();
		glAttachShader(newShaderProgram, vertexShader);
		glAttachShader(newShaderProgram, fragShader);
		glLinkProgram(newShaderProgram);
		int linkSuccess;
		glGetProgramiv(newShaderProgram, GL_LINK_STATUS, &linkSuccess);
		if (!linkSuccess)
		{
			glGetProgramInfoLog(newShaderProgram, 512, NULL, infoLog);
			fprintf(stderr, "Failed to link shaders. Error: %s\n", infoLog);
			return -1;
		}
		else
		{
			GLint mvpLoc = glGetUniformLocation(newShaderProgram, "mvp");
			GLint mvLoc = glGetUniformLocation(newShaderProgram, "mv");
			GLint mvnLoc = glGetUniformLocation(newShaderProgram, "mvn");
			
			GLint diffuseAmbientLocation = glGetUniformLocation(newShaderProgram, "DiffuseAmbientColor");
			GLint specularColorLocation = glGetUniformLocation(newShaderProgram, "SpecularColor");
			GLint specularShininessLocation = glGetUniformLocation(newShaderProgram, "SpecularShininess");
			
			GLint ambientIntensityLocation = glGetUniformLocation(newShaderProgram, "AmbientLightIntensity");
			GLint lightIntensityLoc = glGetUniformLocation(newShaderProgram, "LightIntensity");
			GLint lightPositionLoc = glGetUniformLocation(newShaderProgram, "LightPosition");
			GLint cameraPositionLoc = glGetUniformLocation(newShaderProgram, "CameraPosition");

			if (mvpLoc == -1 || mvLoc == -1 || mvnLoc == -1 ||
				diffuseAmbientLocation == -1 || specularColorLocation == -1 || 
				specularShininessLocation == -1 || ambientIntensityLocation == -1 ||
				lightIntensityLoc == -1 || lightPositionLoc == -1 || cameraPositionLoc == -1)
			{
				fprintf(stderr, "Could not get a uniform location.\n");
				return -1;
			}
			else
			{
				MVPUniformLocation = mvpLoc;
				MVUniformLocation = mvLoc;
				MVNUniformLocation = mvnLoc;
				
				DiffuseAmbientColorLocation = diffuseAmbientLocation;
				SpecularColorLocation = specularColorLocation;
				SpecularShininessLocation = specularShininessLocation;

				AmbientLightIntensityLocation = ambientIntensityLocation;
				LightIntensityLocation = lightIntensityLoc;
				LightPositionLocation = lightPositionLoc;
				CameraPositionLocation = cameraPositionLoc;

				ShaderProgram = newShaderProgram;
			}
		}
	}
	else
	{
		return -1;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);

	return 0;

}
