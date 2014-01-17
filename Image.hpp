/* ----------------------------------------------------------------
   name:           Image.hpp
   purpose:        texturing tutorial 
   'introduction to computer graphics' 
   winter term 2012/2013, assignment 4
   version:	   SKELETON CODE
   TODO:           nothing (see Image.cpp)
   author:         katrin lang
   computer graphics
   tu berlin
   ------------------------------------------------------------- */

#pragma once

#ifdef __APPLE__ 
#include <GL/freeglut.h>
#elif _WIN32
#include "win32/GL/freeglut.h"
#else
#include <GL/freeglut.h>
#endif

#include <vector>
#include <string>

#include "glm/glm.hpp"

class Image{

public:

  // constructors
  Image();
  Image(int width, int height);
  Image(const std::string& filename);

  // destructor
  ~Image();

  // load image from file
  void load(const std::string& filename);

  // set modulation
  void setModulation(GLuint modulation);
  // set texture filter
  void setMinFilter(GLuint min);
  void setMagFilter(GLuint mag);

  // bind/unbind texture
  void bind();
  void unbind();

  // generate OpenGL texture
  void generateTexture();

  // draw in texture
  void paint(float x, float y);

  // erase drawing from texture
  void erase(float x, float y);

protected:

  // image data
  std::vector<glm::vec4> data;
  // dimensions
  int width;
  int height;

  GLuint textureID;

  //texturing parameters
  GLuint wrap;
  GLuint mag;
  GLuint min;
  GLuint modulate;

  // read a pixel from image
  glm::vec4 get(unsigned int x, unsigned int y);

  // parse ppm format
  void loadPPM(const std::string& filename);
};
