#include "Material.h"
#include <sstream>
#include <fstream>
#include <iostream>

Material::Material()
{
	this->reflecting = true;
	this->usetexture = false;
	this->bumpmap = false;
}

Material::Material(vec3 colour){
	this->colour = colour;
	this->reflecting = false;
	this->usetexture = false;
	this->bumpmap = false;
}

Material::~Material()
{
	this->texture.clear();
}

void Material::loadPPM(const std::string& filename){
	this->reflecting = false;
	this->usetexture = true;

	ifstream file(filename.c_str(), ios::binary);

	if (!file.is_open()){
		cerr << "opening file " << filename << " failed" << endl;
		return;
	}

	// grab first two chars of the file and make sure that it has the
	// correct magic cookie for a raw PPM file.
	string magic;
	getline(file, magic);
	if (magic.substr(0, 2) != "P6"){
		cerr << "File " << filename << " is not a raw PPM file" << endl;
		return;
	}

	// grab the three elements in the header (width, height, maxval).
	string dimensions;
	do{
		getline(file, dimensions);
	} while (dimensions[0] == '#');

	stringstream(dimensions) >> texwidth >> texheight;

	string max;
	getline(file, max);
	int maxValue;
	stringstream(max) >> maxValue;
	// grab all the image data in one fell swoop.
	vector<char> raw(texwidth*texheight * 3);
	file.read(&raw[0], raw.capacity());
	file.close();

	texture.resize(texwidth*texheight);
	for (int y = 0; y < texheight; y++){
		for (int x = 0; x < texwidth; x++){
			texture[y*texwidth + x] = vec4((unsigned char)raw[(texheight - y - 1) * texwidth * 3 + 3 * x], (unsigned char)raw[(texheight - y - 1) * texwidth * 3 + 3 * x + 1], (unsigned char)raw[(texheight - y - 1) * texwidth * 3 + 3 * x + 2], maxValue);
			texture[y*texwidth + x] /= maxValue;
			//cout << data[i].r << " " + data[i].g << " " + data[i].b << " " + data[i].a << endl;
		}
	}

	raw.clear();

	std::cout << "Image " << filename << " loaded. width=" << texwidth << " height=" << texheight << endl;
}
