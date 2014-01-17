/* ----------------------------------------------------------------
   name:           Context.cpp
   purpose:        GL context creation, windowing, GLUT stuff
   version:	   SKELETON CODE
   TODO:           nothing 
   author:         katrin lang
   computer graphics
   tu berlin
   ------------------------------------------------------------- */

#include <iostream>
 
#ifdef __APPLE__ 
  #include <GL/glew.h>
#include <GL/freeglut.h>
#elif _WIN32
  #include "win32/GL/glew.h"
#include "win32/GL/freeglut.h"
#else
  #include <GL/glew.h>
#include <GL/freeglut.h>
#endif

#include "glm/glm.hpp"

#include "Context.hpp"
#include "Texture.hpp"

using namespace std;
using namespace glm;

// screen size
static vec2 screen= vec2(1024, 512);


// initial window position
static const vec2 position= vec2(100, 100);

// gap between subwindows
static const int GAP= 3;

// window title
static const string title= "cg1 assignment 4 - texturing";

// light and material
GLfloat Context::materialAmbient[]= {0.5, 0.5, 0.5, 1.0};
GLfloat Context::materialSpecular[]= {0.3, 0.3, 0.3, 1.0};
GLfloat Context::materialShininess[]= { 3.0 };
GLfloat Context::lightModelAmbient[]= { 0.3, 0.3, 0.3 };


// windows
static GLuint mainWindow, textureWindow, worldWindow;


// display callback for GLUT
void Context::display(void){

  // select main window
  glutSetWindow(mainWindow);

  // clear color and depth buffer
  glClearColor(0.8, 0.8, 0.8, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  
  glutSwapBuffers();

  // select texture window
  glutSetWindow(textureWindow);
  // request redisplay
  glutPostRedisplay();

  // select world window
  glutSetWindow(worldWindow);
  // request redisplay
  glutPostRedisplay();
}

// reshape-callback for GLUT
static void reshape(int width, int height){

  // select main window
  glutSetWindow(mainWindow);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, width, height);
  gluOrtho2D(0, width, 0, height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glutPostRedisplay();

  screen= vec2(width, height);

  width-=3*GAP; height-=2*GAP;
  width/=2;

  // select texture window
  glutSetWindow(textureWindow);
  glutPositionWindow(GAP, GAP); 
  glutReshapeWindow(width, height);
  Texture::reshape(width, height);
  // request redisplay
  glutPostRedisplay();

  // select world window
  glutSetWindow(worldWindow);
  glutPositionWindow(width+2*GAP, GAP);
  glutReshapeWindow(width, height);
  World::reshape(width, height);
  // request redisplay
  glutPostRedisplay();
}

// create (sub-) windows
static void createWindows(void){

  glutInitWindowPosition(position.x, position.y);
  glutInitWindowSize(screen.x, screen.y);
  mainWindow= glutCreateWindow(title.c_str());
  glutDisplayFunc(Context::display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(Common::keyPressed);

  float subWidth= (screen.x - 3*GAP)/2;
  float subHeight= screen.y - 2*GAP;

  glutSetOption(GLUT_RENDERING_CONTEXT, GLUT_USE_CURRENT_CONTEXT);

  textureWindow= glutCreateSubWindow(mainWindow, GAP, GAP, subWidth, subHeight);
  glutDisplayFunc(Texture::display);
  glutReshapeFunc(Texture::reshape);
  glutMouseFunc(Texture::mousePressed);
  glutMotionFunc(Texture::mouseDragged);
  glutPassiveMotionFunc(Texture::mouseMoved);
  glutCreateMenu(Texture::menu);
  glutKeyboardFunc(Common::keyPressed);
  for(int i= 0; i<Texture::numOptions; i++) glutAddMenuEntry(Texture::menuText[i].c_str(), Texture::menuOptions[i]);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  worldWindow= glutCreateSubWindow(mainWindow, subWidth + 2*GAP, GAP, subWidth, subHeight);
  glutDisplayFunc(World::display);
  glutReshapeFunc(World::reshape);
  glutMouseFunc(World::mousePressed);
  glutMotionFunc(World::mouseDragged);
  glutCreateMenu(World::menu);
  glutKeyboardFunc(Common::keyPressed);
  for(int i= 0; i<World::numOptions; i++) glutAddMenuEntry(World::menuText[i].c_str(), World::menuOptions[i]);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

}

// initialize OpenGL context
// XXX: NEEDS TO BE IMPLEMENTED
void Context::init(int argc, char **argv){

  // create window with glut
  glutInit(&argc, argv);

  createWindows();

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  // general GL settings
  glEnable(GL_LIGHT_MODEL_LOCAL_VIEWER);
  glShadeModel(GL_SMOOTH);

  // enable normalization of vertex normals
  glEnable(GL_NORMALIZE);
 
  // some normals of our example models are flipped :/
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1);

GLenum err = glewInit();
  if(err != GLEW_OK){
    cerr << "GLEW not available! That means no environment mapping for you" << endl;
  }

  // some output to console
  cout << "--------------------------------------------\n";
  cout << " cg1_ex4 texturing                          \n";
  cout << "                                            \n";
  cout << " keyboard:                                  \n";
  cout << " c/C: match camera position in foto         \n";
  cout << " s/S: scale Model                           \n";
  cout << " r: reset all transformations               \n";
  cout << " q/Q: quit program                          \n";
  cout << "                                            \n";
  cout << " mouse:                                     \n";
  cout << " left click+drag: rotation                  \n";
  cout << " ALT+left click+drag: scale object          \n";
  cout << " CTRL+left click+drag: shift xy             \n";
  cout << " SHIFT+left click+drag: shift z             \n";
  cout << " right click: window-specific menus         \n";
  cout << "--------------------------------------------\n";

  glutMainLoop();
}
