#include <cmath>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>
#include <limits>
#include <ctime>

#include "Ray.h"
#include "Mesh.h"
#include "Scene.h"
#include "Hitresult.h"

#include <omp.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;
using namespace std;
#ifdef __APPLE__ 
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

Scene* scene = new Scene();

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
std::vector<vec3> hitpoints;
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

void save() {
	string filename = "renderedimage.ppm";
	cout << "Output to " << filename << "\n";
	std::ofstream f;
	f.open(filename.c_str(), std::ios_base::trunc);
	f << "P3\n" << _sample_width << " " << _sample_height << "\n255\n";
	for (int y = _sample_height - 1; y; y--)
		for (int x = 0; x < _sample_width; x++)	{
			int i = y * _sample_width + x;
			f << (int)(rayTracedImage[i].x * 255) << " " << (int)(rayTracedImage[i].y * 255) << " " << (int)(rayTracedImage[i].z * 255) << " ";
		}
	f.close();
	cout << "Output complete\n";
}

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
	vec3 start = glm::unProject(vec3(resx, resy, 0), modelview, projection, glm::make_vec4(viewport));
	vec3 end = glm::unProject(vec3(resx, resy, 1), modelview, projection, glm::make_vec4(viewport));
	rays.push_back(Ray(start, glm::normalize(end - start)));
}

// Ray trace the scene
void ray_trace()
{
	// Number of samples in x and y direction given the sampling factor
	int w = (int)(_sample_factor * (float)_win_w);
	int h = (int)(_sample_factor * (float)_win_h);

	_sample_width = w;
	_sample_height = h;

	std::cout << "raycast: w=" << w << " h=" << h << std::endl;

	rayTracedImage.clear();
	hitpoints.clear();
	rayTracedImage.resize(w*h, vec3(0, 1, 0));
	cout << "Begin raytracing\n";
	clock_t start = std::clock();
	for (float y = 0; y < _win_h; y += (1 / _sample_factor))
		for (float x = 0; x < _win_w; x += (1 / _sample_factor))
			create_primary_rays(rays, (int)x, (int)y);
	// TODO : write the samples with the correct color (i.e raytrace)
	mat4 mvinv = glm::inverse(modelview);
	std::srand(15);
	omp_lock_t lock;
	omp_init_lock(&lock);
	#pragma omp parallel for
	for (int coord = 0; (unsigned int) coord < rays.size(); coord++){ 
			Ray* ray = &rays.at(coord);
			ray->o = (vec3)(mvinv*vec4(ray->o, 1));
			ray->d = (vec3)(mvinv*vec4(ray->d, 0));
			Hitresult* hit = scene->intersectscene(&rays.at(coord));
			vec3 fragcolour;
			if (hit == nullptr)
				fragcolour = vec3(0.6f, 0.6f, 0.6f);
			else {
				fragcolour = hit->ambcolour;
				omp_set_lock(&lock); //Concurrent modification of the hitpoints is troubling
				hitpoints.push_back((vec3)(modelview*vec4(hit->reflectray->o, 1)));
				omp_unset_lock(&lock);
				delete hit;
			}
			rayTracedImage[coord] = fragcolour;
		}
	omp_destroy_lock(&lock);
	clock_t stop = std::clock();
	cout << "Rendered in " << stop - start << " milliseconds\n";
	rays.clear();
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
	for (unsigned int i = 0; i < hitpoints.size(); i++){
		vec3 point = hitpoints.at(i);
		glColor3ub(0, 255, 0);
		glBegin(GL_POINTS); {
			glVertex3f(point.x, point.y, point.z);
		}glEnd();
	}
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
	scene->renderscenegl();
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
		save();
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
		//When testing, then turn off centralization and distance normalization in the model loader
		*/
	Mesh* mesh = new Mesh();
	mesh->loadOff("scenedata/drei.off");
	Ray* ray = new Ray(vec3(0, 0, 0), vec3(1, 0, 0));
	Hitresult* hit = mesh->intersectpolygon(mesh->polygon[0], ray);
	if (hit == nullptr)
		std::cout << "No hit\n";
	else{
		std::cout << "Distance " << hit->distance << "\n";
		std::cout << "Hitpoint " << hit->reflectray->o.x << " " << hit->reflectray->o.y << " " << hit->reflectray->o.z << "\n";
		std::cout << "Reflection " << hit->reflectray->d.x << " " << hit->reflectray->d.y << " " << hit->reflectray->d.z << "\n";
	}
	scene->loadscenedata();
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

