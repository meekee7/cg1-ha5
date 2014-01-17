/* ----------------------------------------------------------------
   name:           context.h
   purpose:        GL context class declaration, prototypes of GLUT callbacks
   version:	   SKELETON CODE
   TODO:           nothing
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

#include <string>
#include "glm/glm.hpp"

namespace Context{

  // material ambient color
  extern GLfloat materialAmbient[4];
  // material specular color
  extern GLfloat materialSpecular[4];
  // material shininess
  extern GLfloat materialShininess[1];
  // ambient color
  extern GLfloat lightModelAmbient[3];

  // intialization                                             
  void init(int argc, char **argv);  
  
  // redisplay all windows
  void display(void);
};
