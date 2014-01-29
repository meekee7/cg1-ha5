#include <cmath>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>
#include <limits>


#include "Ray.h"
#include "Mesh.h"
#include "Hitresult.h"

#include <omp.h>
#include <glm/glm.hpp>

using namespace glm;

#ifdef __APPLE__ 
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

Mesh* mesh = new Mesh();
/*float determinant(mat3 mat){
	return (mat[0][0] * mat[1][1] * mat[2][2] + mat[0][1] * mat[1][2] * mat[2][0] + mat[0][2] * mat[1][0] * mat[2][1])
	- (mat[0][2] * mat[1][1] * mat[2][0] + mat[0][1] * mat[1][0] * mat[2][2] + mat[0][0] * mat[1][2] * mat[2][1]);
	}

	float determinant(mat2 mat){
	return mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
	}

	vec3 cross(vec3 v1, vec3 v2){
	return vec3(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
	}

	float dot(vec3 v1, vec3 v2){
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	//Erste drei Einträge Schnittpunkt, letzer Eintrag Distanz, 0,0,0,-1 wenn kein Schnitt
	vec4 rayhittriangle(Ray ray, vec3 trix, vec3 triy, vec3 triz){
	{vec3 u = triy - trix; //http://cgvr.cs.uni-bremen.de/teaching/cg2_10/folien/07_raytracing_2.pdf
	vec3 v = triz - trix;
	vec3 w = ray.o - trix;
	vec3 crossdv = cross(ray.d, v);
	vec3 crosswu = cross(w, u);
	vec3 hitpoint = (1 / dot(crossdv, u)) * vec3(dot(crosswu, v), dot(crossdv, w), dot(crosswu, ray.d));
	if (0 < hitpoint.x < 1 && 0 < hitpoint.z < 1 && hitpoint.x + hitpoint.z <= 1)
	return vec4(hitpoint, 1 - hitpoint.x - hitpoint.z);
	else
	return vec4(0, 0, 0, -1);}
	{//Vorlesungsfolien
	float det = 1.0f / determinant(mat3(-ray.d, triy - trix, triz - trix));
	float t = det*dot(cross(ray.o - trix, triy - trix), triz - trix);
	float u = det*dot(cross(ray.d, triz - trix), ray.o - trix);
	float v = det*dot(cross(ray.o - trix, triy - trix), ray.d);
	if (0 < u < 1 && 0 < v<1 && t>0)
	return vec4(u, v, u + v, t);
	else
	return vec4(0, 0, 0, -1);
	}
	{vec3 normal = cross(triz - trix, triy - trix); //http://uninformativ.de/bin/RaytracingSchnitttests-76a577a-CC-BY.pdf
	float d = dot(normal, trix);
	float rn = dot(normal, ray.d);
	if (rn == 0.0f) // If we do not catch division by zero, then we get invalid results
	return vec4(0, 0, 0, -1);
	float alpha1 = (d - dot(ray.o, normal)) / rn;
	vec3 q = ray.o + alpha1*ray.d;
	vec3 b = triy - trix;
	vec3 c = triz - trix;
	float bb = dot(b, b);
	float bc = dot(b, c);
	float cc = dot(c, c);
	float D = 1.0f / (cc*bb - bc*bc);
	float bbd = bb * D;
	float bcd = bc * D;
	float ccd = cc * D;
	vec3 ubeta = (ccd * b) - (c * bcd);
	float kbeta = dot(-trix, ubeta);
	vec3 ugamma = (c *bbd) - (b*bcd);
	float kgamma = dot(-trix, ugamma);
	float beta = dot(ubeta, q) + kbeta;
	float gamma = dot(ugamma, q) + kgamma;
	float alpha = 1 - beta - gamma;
	if (alpha1 <= 0.0f || beta < 0.0f || gamma < 0.0f || alpha < 0.0f)
	return vec4(0, 0, 0, -1);
	else
	return vec4(q, alpha1);
	}
	}*/

// global variables //////////////////////////////////////////
int _id_window, _id_screen, _id_world;
int _win_w = 256;
int _win_h = 256;
int _win_gap = 8;

vec3 eye(0, 5, 20);
vec3 up(0, 1, 0);
vec3 center(0, 0, 0);
float fovy = 60;
float zNear = 0.1f;
float zFar = 100.0;

vec4 _clear_col(0.5, 0.5, 0.5, 1.0);

mat4 projection, modelview, modelview2, modelview2_inv;
int viewport[4];

float _view_oldx = 0;
float _view_oldy = 0;
float _view_rotx = 0;
float _view_roty = 0;
bool _view_dofovy = false;
bool _view_doshift = false;
bool _view_motion = false;

float _world_oldx = 0;
float _world_roty = -35;


std::vector<Ray> rays;
std::vector<vec3> rayTracedImage;
GLuint rayTracedImageId = 0;

float _sample_factor = 1;
int _sample_width = 0;
int _sample_height = 0;

int _rays_disp_step = 10;
bool _auto_vis = true;

int _recursions = 0;

enum vis_mode { vis_default, vis_opengl, vis_isecs, vis_lods, vis_N };
int _vis_mode = vis_default;

char* _vis_names[vis_N] = { "raytraced image", "opengl shaded" };


/*********************************************************************/
// raytracing
/*********************************************************************/
void clear_rays()
{
	rays.clear();
	rayTracedImage.clear();
}

// Create rays for each sample of the image
void create_primary_rays(std::vector<Ray>& rays, int resx, int resy)
{
	/*mat4 mvi = glm::inverse(modelview);
	mat4 mvt = glm::transpose(modelview);
	mat4 mv2t = glm::transpose(modelview2);
	for (int x = 0; x < resx; x++)
	for (int y = 0; y < resy; y++){ //For values see draw_camera
	float planew = 1;
	float planex = ((float)x / resx) * 2 - 1;
	float planey = ((float)y / resy) * 2 - 1;
	float planez = -2;
	vec4 planeposh(planex, planey, planez, planew);
	vec4 directionh = planeposh;
	planeposh = mvi * planeposh;
	directionh = mvt * directionh;
	planeposh = modelview2_inv * planeposh;
	directionh = mv2t * directionh;
	vec3 direction = vec3(directionh.x / directionh.w, directionh.y / directionh.w, direction.z / directionh.w);
	vec3 planepos = vec3(planeposh.x / planeposh.w, planeposh.y / planeposh.w, planeposh.z / planeposh.w);
	glm::normalize(direction);
	rays.push_back(Ray(planepos, direction));
	}*/
	// TODO!
	double startX, startY, startZ, endX, endY, endZ;
	// Get the Projection and Model View Matrices for gluUnProject
	GLdouble modelViewMat[16];
	GLdouble projViewMat[16];
	int viewport[4];
	vec3 startVec;
	vec3 endVec;

	glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMat);
	glGetDoublev(GL_PROJECTION_MATRIX, projViewMat);
	glGetIntegerv(GL_VIEWPORT, viewport);

	Ray ray;
	// Finds the Coordinate of the object that is projected to the Image-Plane by the Ray 
	for (float x = 0; x < _win_w; x += _win_w / resx){
		for (float y = 0; y < _win_h; y += _win_h / resy){
			gluUnProject(x, y, 0.0, modelViewMat, projViewMat, viewport, &startX, &startY, &startZ);
			gluUnProject(x, y, 1.0, modelViewMat, projViewMat, viewport, &endX, &endY, &endZ);

			startVec.x = startX;
			startVec.y = startY;
			startVec.z = startZ;

			endVec.x = endX;
			endVec.y = endY;
			endVec.z = endZ;

			// direction ray
			ray.d = endVec - startVec;
			ray.d = glm::normalize(ray.d);

			ray.o = startVec;
			rays.push_back(ray);
		}
	}

}

// Ray trace the scene
void ray_trace()
{
	// Number of samples in x and y direction given the sampling factor
	int w = (int)_sample_factor * _win_w;
	int h = (int)_sample_factor * _win_h;

	_sample_width = w;
	_sample_height = h;

	std::cout << "raycast: w=" << w << " h=" << h << std::endl;

	create_primary_rays(rays, w, h);

	rayTracedImage.clear();
	rayTracedImage.resize(w*h, vec3(0, 1, 0));

	// TODO : write the samples with the correct color (i.e raytrace)
	/*#pragma omp parallel for
		for (int coord = 0; coord < w*h; coord++){
		Hitresult* hit = mesh->intersectModel(&rays.at(coord));
		if (hit == nullptr)
		rayTracedImage[coord] = vec3(0, 0, 0);
		else
		rayTracedImage[coord] = vec3(0, 1, 0);
		delete hit;
		}*/
#pragma omp parallel for
	for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++){
			int coord = x + y*h;
			Hitresult* hit = mesh->intersectModel(&rays.at(coord));
			if (hit == nullptr)
				rayTracedImage[coord] = vec3(0, 0, 0);
			else
				rayTracedImage[coord] = vec3(0, 1, 0);
			delete hit;
		}
	/*for (int i = 0; i < w; i++)
	{
	for (int j = 0; j < h; j++)
	{
	if ((i + j) % 2 == 0)
	{
	rayTracedImage[j*w + i] = vec3(1, 1, 0);
	}
	}
	}*/

	// Create an openGL texture if it doesn't exist allready
	if (!rayTracedImageId)
	{
		glGenTextures(1, &rayTracedImageId);
		glBindTexture(GL_TEXTURE_2D, rayTracedImageId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	// Feed with raytraced image data
	// This texture is displayed on the raytraced image view (here filtering to GL_NEAREST to see the pixels)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, &rayTracedImage[0]);
}

// Draw the rays shot on the scene
void draw_rays()
{
}

// drawing utilities //////////////////////////////////
int get_vis_mode()
{
	bool can_show_image = (!_view_motion && !rayTracedImage.empty());
	return !can_show_image && _vis_mode == vis_default ? vis_opengl : _vis_mode;
}

void draw_camera()
{
	const vec3 c[4] =
	{
		vec3(1, 1, -2),
		vec3(1, -1, -2),
		vec3(-1, -1, -2),
		vec3(-1, 1, -2)
	};

	glBegin(GL_QUADS);
	glColor3f(0.2f, 0.2f, 0.2f);
	for (int i = 0; i < 4; i++)
	{
		glVertex3fv(&c[i][0]);
	}
	glEnd();

	glBegin(GL_LINES);
	glColor3f(1, 1, 0);
	for (int i = 0; i < 4; i++)
	{
		glVertex3f(0, 0, 0);
		glVertex3fv(&c[i][0]);
	}
	glEnd();
}

void full_screen_quad(){
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_CULL_FACE);

	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3i(-1, -1, -1);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3i(1, -1, -1);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3i(1, 1, -1);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3i(-1, 1, -1);

	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void draw_raytraced_image()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, rayTracedImageId);
	glDisable(GL_LIGHTING);
	full_screen_quad();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
}

void draw_string(int x, int y, const std::string& str)
{
	void* font = GLUT_BITMAP_HELVETICA_12;
	const int height = 15;
	const int tab = 18;

	vec3 colors[4] = { vec3(0, 0, 0), vec3(0.0, 1, 0.5), vec3(0.9, 0.3, 0.1), vec3(1, 1, 1) };

	glColor3fv(&colors[0][0]);

	glRasterPos2i(x, y);
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '\n')
		{
			y += height;
			glRasterPos2i(x, y);

			continue;
		}

		if (str[i] == '\t')
		{
			int p[4];
			glGetIntegerv(GL_CURRENT_RASTER_POSITION, p);

			int cx = ((p[0] + tab - 1) / tab) * tab;
			glRasterPos2i(cx, y);
		}

		if (str[i] == '$')
		{
			++i;

			int k = (int)str[i] - '0';
			if (k < 0 || k >= 4) k = 0;
			glColor3fv(&colors[k][0]);

			// update raster color, by setting current raster position
			int p[4];
			glGetIntegerv(GL_CURRENT_RASTER_POSITION, p);
			glRasterPos2i(p[0], y);

			continue;
		}

		glutBitmapCharacter(font, str[i]);
	}
}

// window drawing ///////////////////////////////////////

void main_reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, height, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	clear_rays();

	_win_w = (width - 3 * _win_gap) / 2;
	_win_h = height - 2 * _win_gap - 120;

	glutSetWindow(_id_screen);
	glutPositionWindow(_win_gap, _win_gap);
	glutReshapeWindow(_win_w, _win_h);

	glutSetWindow(_id_world);
	glutPositionWindow(_win_gap + _win_w + _win_gap, _win_gap);
	glutReshapeWindow(_win_w, _win_h);
}

void main_display()
{
	glClearColor(0.7f, 0.7f, 0.7f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	std::ostringstream text;
	text << "mouse \t: rotate scene, mouse+shift: change field of view\n";
	text << "space \t: raytrace scene\n";
	text << "tab   \t: change visualization (current: $1" << _vis_names[get_vis_mode()] << "$0)\n";

	std::ostringstream text2;
	text2 << "key s,S\t: +/- sampling factor $2" << _sample_factor << "$0\n";
	text2 << "key i\t: render image to file\n";

	draw_string(_win_gap, _win_gap + _win_h + _win_gap + 12, text.str());
	draw_string(2 * _win_gap + _win_w, _win_gap + _win_h + _win_gap + 12, text2.str());

	glutSwapBuffers();
}

void draw_scene_openGL()
{
	glPushMatrix();
	glMultMatrixf(&modelview[0][0]);
	glEnable(GL_LIGHTING);

	// TODO :
	// Draw the preview scene here
	glBegin(GL_TRIANGLES);
	glVertex3f(0, 0, 0);
	glVertex3f(10, 0, 0);
	glVertex3f(0, 10, 0);
	glEnd();

	glPopMatrix();
}

void world_reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, (float)width / height, 1.0, 256.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, -5.0, -30.0);
	glRotatef(_world_roty, 0.0, 1.0, 0.0);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
}

void world_display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	glMultMatrixf(&modelview2_inv[0][0]);

	glDisable(GL_LIGHTING);
	draw_camera();

	glMultMatrixf(&modelview[0][0]);

	draw_rays();

	draw_scene_openGL();

	glutSwapBuffers();
}

void screen_reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (float)width / height, zNear, zFar);
	glGetFloatv(GL_PROJECTION_MATRIX, &projection[0][0]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2]);
	glGetFloatv(GL_MODELVIEW_MATRIX, &modelview2[0][0]);
	modelview2_inv = glm::inverse(modelview2);
	glRotatef(_view_rotx, 1, 0, 0);
	glRotatef(_view_roty, 0, 1, 0);
	glGetFloatv(GL_MODELVIEW_MATRIX, &modelview[0][0]);

	glClearColor(_clear_col[0], _clear_col[1], _clear_col[2], _clear_col[3]);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

}

void screen_display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int vis = get_vis_mode();
	// Diplay the ray-traced image
	if (vis == vis_default)
	{
		draw_raytraced_image();
	}
	else
	{
		draw_scene_openGL();
	}

	glutSwapBuffers();
}

void redisplay_all()
{
	glutSetWindow(_id_window);
	glutPostRedisplay();

	glutSetWindow(_id_world);
	world_reshape(_win_w, _win_h);
	glutPostRedisplay();

	glutSetWindow(_id_screen);
	screen_reshape(_win_w, _win_h);
	glutPostRedisplay();
}

// gui interaction /////////////////////////////////////

void screen_mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT && state == GLUT_DOWN)
	{
		_view_motion = true;

		_view_oldx = (float)x;
		_view_oldy = (float)y;

		_view_dofovy = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;
		_view_doshift = (glutGetModifiers() & GLUT_ACTIVE_ALT) != 0;
	}

	if (state == GLUT_UP)
	{
		_view_motion = false;
		ray_trace();
	}

	redisplay_all();
}

void screen_motion(int x, int y)
{
	if (_view_motion)
	{
		if (_view_dofovy)
		{
			fovy += 1.0f * (y - _view_oldy);
		}
		else if (_view_doshift)
		{
			center[0] += -0.3f * (x - _view_oldx);
			center[1] += 0.3f * (y - _view_oldy);
		}
		else
		{
			_view_roty += -1.0f * (x - _view_oldx);
			_view_rotx += 1.0f * (y - _view_oldy);
		}

		_view_oldx = (float)x;
		_view_oldy = (float)y;
	}

	redisplay_all();
}

void screen_passive_motion(int x, int y)
{
	if (x < 0 || y < 0 || x >= _win_w || y >= _win_h)
	{
		return;
	}
}

void screen_menu(int value)
{
	redisplay_all();
}

void world_mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT && state == GLUT_DOWN)
	{
		_world_oldx = (float)x;
	}
}

void world_motion(int x, int y)
{
	_world_roty += -1.0f * ((float)x - _world_oldx);
	_world_oldx = (float)x;

	redisplay_all();
}

void world_menu(int value)
{
	redisplay_all();
}

void main_keyboard(unsigned char key, int x, int y)
{
	if ((int)key == 9)
	{
		_view_motion = !_view_motion;
	}
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case 'r':
		ray_trace();
		break;
	case 'i':
		break;
	case 's':
		_sample_factor *= 2;
		std::cout << "sampling factor: " << _sample_factor << std::endl;
		break;
	case 'S':
		_sample_factor *= 0.5;
		std::cout << "sampling factor: " << _sample_factor << std::endl;
		break;
	}

	redisplay_all();
}

void idle()
{
}

int main(int argc, char** argv)
{
	/*for (int x = 0; x < 7; x++){
		for (int y = 0; y < 10; y++)
		cout << (x * 7 + y) << " ";
		cout << "\n";
		}
		//Test some stuff
		//Mesh* mesh = new Mesh(); //When testing, then turn off centralization and distance normalization in the model loader
		*/mesh->loadOff("scenedata/drei.off");
Ray* ray = new Ray(vec3(0, 0, 0), vec3(1, 0, 0));
Hitresult* hit = mesh->intersectpolygon(mesh->polygon[0], ray);
if (hit == nullptr)
std::cout << "No hit\n";
else{
	std::cout << "Distance " << hit->distance << "\n";
	std::cout << "Hitpoint " << hit->reflectray->o.x << " " << hit->reflectray->o.y << " " << hit->reflectray->o.z << "\n";
	std::cout << "Reflection " << hit->reflectray->d.x << " " << hit->reflectray->d.y << " " << hit->reflectray->d.z << "\n";
}
// Init OpenGL stuffs
glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
glutInitWindowSize(1024, 600);
glutInitWindowPosition(50, 50);
glutInit(&argc, argv);

// Create main window
_id_window = glutCreateWindow("cg1 ex5 ws13/14 - raytracing");
glutReshapeFunc(main_reshape);
glutDisplayFunc(main_display);
glutKeyboardFunc(main_keyboard);

glutIdleFunc(idle);

// Create world window
_id_world = glutCreateSubWindow(_id_window, _win_gap, _win_gap, _win_w, _win_h);
glutReshapeFunc(world_reshape);
glutDisplayFunc(world_display);
glutMouseFunc(world_mouse);
glutMotionFunc(world_motion);
glutKeyboardFunc(main_keyboard);
glutCreateMenu(world_menu);
glutAttachMenu(GLUT_RIGHT_BUTTON);

// Create view window
_id_screen = glutCreateSubWindow(_id_window, _win_gap + _win_w + _win_gap, _win_gap, _win_w, _win_h);
glutReshapeFunc(screen_reshape);
glutDisplayFunc(screen_display);
glutMouseFunc(screen_mouse);
glutMotionFunc(screen_motion);
glutPassiveMotionFunc(screen_passive_motion);
glutKeyboardFunc(main_keyboard);
glutCreateMenu(screen_menu);
redisplay_all();

glutMainLoop();

return 0;
}

