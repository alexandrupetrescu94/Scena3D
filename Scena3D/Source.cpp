// codurile pot fi gasite pe site-ul http://www.opengl.org/resources/code/samples/glut_examples/advanced/advanced.html
/* projshadow.c - by Tom McReynolds, SGI */

/* Rendering shadows using projective shadows. */

#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>

GLfloat x0 = 50.0, y0 = 150.0, z0 = 300.0;
GLfloat xref = 0.0, yref = 150.0, zref = 0.0;
GLfloat Vx = 0.0, Vy = 1.0, Vz = 0.0;

GLfloat xwMin = -30.0, ywMin = -30.0, xwMax = 30.0, ywMax = 30.0;

GLfloat dnear = 1.0, dfar = 40.0;

// angle of rotation for the camera direction
float angle=0.0;
// actual vector representing the camera's direction
float lx=0.0f,lz=-1.0f;
/* Create a single component texture map */
GLfloat * make_texture(int maxs, int maxt)
{
	int s, t;
	static GLfloat *texture;

	texture = (GLfloat *)malloc(maxs * maxt * sizeof(GLfloat));
	for (t = 0; t < maxt; t++) {
		for (s = 0; s < maxs; s++) {
			texture[s + maxs * t] = ((s >> 4) & 0x1) ^ ((t >> 4) & 0x1);
		}
	}
	return texture;
}

enum {
	SPHERE = 1, LIGHT, LEFTWALL, FLOOR
};

enum {
	X, Y, Z, W
};

GLfloat lightpos[] =
{ 50.f, 50.f, -320.f, 1.f };

void redraw(void)
{
	/* material properties for objects in scene */
	static GLfloat wall_mat[] =
	{ 1.f, 1.f, 1.f, 1.f };

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/* Note: wall verticies are ordered so they are all front facing this lets
	me do back face culling to speed things up.  */

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, wall_mat);

	/* floor */
	/* make the floor textured */
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glNormal3f(0.f, 1.f, 0.f);
	glTexCoord2i(0, 0);
	glVertex3f(-100.f, 0.f, -200.f);
	glTexCoord2i(1, 0);
	glVertex3f(-100.f, 0.f, -500.f);
	glTexCoord2i(1, 1);
	glVertex3f(200.f, 0.f, -500.f);
	glTexCoord2i(0, 1);
	glVertex3f(200.f, 0.f, -200.f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	/* walls */

	glBegin(GL_QUADS);
	/* left wall */
	glNormal3f(1.f, 0.f, 0.f);
	glVertex3f(-100.f, 0.f, -200.f);
	glVertex3f(-100.f, 0.f, -500.f);
	glVertex3f(-100.f, 300.f, -500.f);
	glVertex3f(-100.f, 300.f, -200.f);
	glEnd();

	glBegin(GL_QUADS);
	/* right wall */
	glNormal3f(-1.f, 0.f, 0.f);
	glVertex3f(200.f, 0.f, -200.f);
	glVertex3f(200.f, 0.f, -500.f);
	glVertex3f(200.f, 300.f, -500.f);
	glVertex3f(200.f, 300.f, -200.f);

	/* ceiling */
	glNormal3f(0.f, -1.f, 0.f);
	glVertex3f(-100.f, 300.f, -200.f);
	glVertex3f(-100.f, 300.f, -500.f);
	glVertex3f(200.f, 300.f, -500.f);
	glVertex3f(200.f, 300.f, -200.f);

	/* back wall */
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(-100.f, 0.f, -500.f);
	glVertex3f(200.f, 0.f, -500.f);
	glVertex3f(200.f, 300.f, -500.f);
	glVertex3f(-100.f, 300.f, -500.f);

	/* front wall */
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(-100.f, 0.f, -200.f);
	glVertex3f(200.f, 0.f, -200.f);
	glVertex3f(200.f, 300.f, -200.f);
	glVertex3f(-100.f, 300.f, -200.f);
	glEnd();

/* Map */

/* Left Block */
glNormal3f(0.f, -1.f, 0.f);
glVertex3f(-300.f, 0.f, -800.f);
glVertex3f(-100.f, 0.f, -800.f);
glVertex3f(-100.f, 0.f, 100.f);
glVertex3f(-300.f, 0.f, 100.f);

/* Right Block */
glNormal3f(0.f, -1.f, 0.f);
glVertex3f(200.f, 0.f, -800.f);
glVertex3f(400.f, 0.f, -800.f);
glVertex3f(400.f, 0.f, 100.f);
glVertex3f(200.f, 0.f, 100.f);
glEnd();

/* Backward Block */
glNormal3f(0.f, -1.f, 0.f);
glVertex3f(-100.f, 0.f, -800.f);
glVertex3f(200.f, 0.f, -800.f);
glVertex3f(200.f, 0.f, -500.f);
glVertex3f(-100.f, 0.f, -500.f);
glEnd();

/* Frontward Block */
glNormal3f(0.f, -1.f, 0.f);
glVertex3f(-100.f, 0.f, -200.f);
glVertex3f(200.f, 0.f, -200.f);
glVertex3f(200.f, 0.f, 100.f);
glVertex3f(-100.f, 0.f, 100.f);
glEnd();


	glPushMatrix();
	glTranslatef(lightpos[X], lightpos[Y], lightpos[Z]);
	glDisable(GL_LIGHTING);
	glColor3f(1.f, 1.f, .7f);
	glCallList(LIGHT);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	glutSwapBuffers();    /* high end machines may need this */
}

void key(unsigned char key, int x, int y)
{
	if (key == '\033')
		exit(0);
}

const int TEXDIM = 256;

void processSpecialKeys(int key, int xx, int yy) {

	float fraction = 0.1f;

	switch (key) {
		case GLUT_KEY_UP :
			xref += lx * fraction;
			zref += lz * fraction;
			break;
		case GLUT_KEY_DOWN :
			xref -= lx * fraction;
			zref -= lz * fraction;
			break;
	}
}

int main(int argc, char *argv[])
{
	GLfloat *tex;
	GLUquadricObj *sphere;


	glutInit(&argc, argv);
	glutInitWindowSize(810,810);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL | GLUT_DOUBLE);
	(void)glutCreateWindow("Scena3D");
	glutDisplayFunc(redraw);
	glutKeyboardFunc(key);
	glutSpecialFunc(processSpecialKeys);

	/* draw a perspective scene */
	glMatrixMode(GL_PROJECTION);
	//glFrustum(-300., 400., 0., 300., 100., -800.); left right bottom top near far
	glFrustum(-100., 100., -100., 100., 200., 1100.); // raportat la observator
	// glMatrixMode(GL_MODELVIEW);
	gluLookAt (x0, y0, z0, xref, yref, zref, Vx, Vy, Vz);
	/* turn on features */
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	/* place light 0 in the right place */
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

	/* remove back faces to speed things up */
	//glCullFace(GL_BACK);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	/* make display lists for sphere and cone; for efficiency */

	glNewList(LIGHT, GL_COMPILE);
	sphere = gluNewQuadric();
	gluSphere(sphere, 5.f, 20, 20);
	gluDeleteQuadric(sphere);
	glEndList();


	/* load pattern for current 2d texture */
	tex = make_texture(TEXDIM, TEXDIM);
	glTexImage2D(GL_TEXTURE_2D, 0, 1, TEXDIM, TEXDIM, 0, GL_RED, GL_FLOAT, tex);
	free(tex);

	glutMainLoop();
	return 0;
}
