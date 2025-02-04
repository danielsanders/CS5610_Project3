#pragma once

#include "cyVector.h"

class Material
{
public:
	Material();

	cyVec4f AmbientDiffuseColor;
	cyVec4f SpecularColor;
	float SpecularShininess;

};

