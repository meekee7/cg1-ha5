#include "Material.h"

Material::Material()
{
	this->reflecting = true;
}

Material::Material(vec3 colour){
	this->colour = colour;
	this->reflecting = false;
}

Material::~Material()
{
}
