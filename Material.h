#include "glm/glm.hpp"
#include <cstring>
#include <vector>
//#include "Scene.h"
using namespace glm;
using namespace std;
#pragma once
class Material
{
public:
	Material();
	Material(vec3 colour);
	~Material();
	vec3 colour;
	bool reflecting;
	bool usetexture;
	bool bumpmap;
	bool celshade;
	int texwidth;
	int texheight;
	std::vector<glm::vec4> texture;
	void loadPPM(const std::string& filename);
};

