/* ----------------------------------------------------------------
   name:           Texture.hpp
   purpose:        
   version:	   SKELETON CODE
   TODO:           nothing (see Texture.cpp)
   author:         katrin lang
   computer graphics
   tu berlin
   ------------------------------------------------------------- */

#pragma once

#include <string>

namespace Common{

  // common keyboard callback
  void keyPressed(unsigned char key, int x, int y);
};

namespace Texture{
  
  extern int menuOptions[];
  extern std::string menuText[];
  extern int numOptions;
  extern glm::vec2 previousMouse; // previous mouse position

  void display(void);
  void reshape(int width, int height);
  void mousePressed(int button, int state, int x, int y);
  void mouseDragged(int x, int y);
  void mouseMoved(int x, int y);
  void menu(int value);
};

namespace World{

  extern int menuOptions[];
  extern std::string menuText[];
  extern int numOptions;
  extern glm::vec2 previousMouse; // previous mouse position
  
  void display(void);
  void reshape(int width, int height);
  void mousePressed(int button, int state, int x, int y);
  void mouseDragged(int x, int y);
  void menu(int value);
};
