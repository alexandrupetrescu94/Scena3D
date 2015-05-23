#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace std;

//Observer Coords
GLfloat x0 = 50.0, y0 = 450.0, z0 = 300.0;
GLfloat xref = 0.0, yref = 300.0, zref = 0.0;
GLfloat Vx = 0.0, Vy = 1.0, Vz = 0.0;
//Projection Coords
GLfloat dnear = 1.0, dfar = 40.0;


GLfloat xwMin = -30.0, ywMin = -30.0, xwMax = 30.0, ywMax = 30.0;
float angle = 0.0; 		// angle of rotation for the camera direction
float lx = 0.0f,lz = -1.0f; // actual vector representing the camera's direction


GLfloat *tex;
GLfloat * make_texture(int maxs, int maxt) {
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


GLUquadricObj *sphereLight;
GLfloat lightpos[] = { -150.f, -150.f, 50.f, 1.f };
enum {
	X, Y, Z, W
};
enum {
	SPHERE = 1, LIGHT, LEFTWALL, FLOOR
};



void build_map(){
	/* Left Block */
    glBegin(GL_QUADS);
    glNormal3f(0.f, -1.f, 0.f);
    glVertex3f(-300.f, 0.f, -800.f);
    glVertex3f(-100.f, 0.f, -800.f);
    glVertex3f(-100.f, 0.f, 100.f);
    glVertex3f(-300.f, 0.f, 100.f);
    glEnd();

    /* Right Block */
    glBegin(GL_QUADS);
    glNormal3f(0.f, -1.f, 0.f);
    glVertex3f(200.f, 0.f, -800.f);
    glVertex3f(400.f, 0.f, -800.f);
    glVertex3f(400.f, 0.f, 100.f);
    glVertex3f(200.f, 0.f, 100.f);
    glEnd();

    /* Backward Block */
    glBegin(GL_QUADS);
    glNormal3f(0.f, -1.f, 0.f);
    glVertex3f(-100.f, 0.f, -800.f);
    glVertex3f(200.f, 0.f, -800.f);
    glVertex3f(200.f, 0.f, -500.f);
    glVertex3f(-100.f, 0.f, -500.f);
    glEnd();

    /* Frontward Block */
    glBegin(GL_QUADS);
    glNormal3f(0.f, -1.f, 0.f);
    glVertex3f(-100.f, 0.f, -200.f);
    glVertex3f(200.f, 0.f, -200.f);
    glVertex3f(200.f, 0.f, 100.f);
    glVertex3f(-100.f, 0.f, 100.f);
    glEnd();
}

void build_house(){
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
	glEnd();
	/* ceiling */
	glBegin(GL_QUADS);
	glNormal3f(0.f, -1.f, 0.f);
	glVertex3f(-100.f, 300.f, -200.f);
	glVertex3f(-100.f, 300.f, -500.f);
	glVertex3f(200.f, 300.f, -500.f);
	glVertex3f(200.f, 300.f, -200.f);
	glEnd();
	/* back wall */
	glBegin(GL_QUADS);
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(-100.f, 0.f, -500.f);
	glVertex3f(200.f, 0.f, -500.f);
	glVertex3f(200.f, 300.f, -500.f);
	glVertex3f(-100.f, 300.f, -500.f);
	glEnd();

	/* front wall 3 parts with door*/
	glBegin(GL_QUADS);
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(-100.f, 0.f, -200.f);
	glVertex3f(0.f, 0.f, -200.f);
	glVertex3f(0.f, 300.f, -200.f);
	glVertex3f(-100.f, 300.f, -200.f);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 130.f, -200.f);
	glVertex3f(50.f, 130.f, -200.f);
	glVertex3f(50.f, 300.f, -200.f);
	glVertex3f(0.f, 300.f, -200.f);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(50.f, 0.f, -200.f);
	glVertex3f(200.f, 0.f, -200.f);
	glVertex3f(200.f, 300.f, -200.f);
	glVertex3f(50.f, 300.f, -200.f);
	glEnd();
}

void renderScene(void)
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/* material properties for objects in scene */
	static GLfloat wall_mat[] = { 1.f, 1.f, 1.f, 1.f };
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, wall_mat);

	build_house();
	build_map();

	glPushMatrix();
	glTranslatef(lightpos[X], lightpos[Y], lightpos[Z]);
	glDisable(GL_LIGHTING);
	glColor3f(1.f, 1.f, .7f);
	glCallList(LIGHT);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	glutSwapBuffers();
}

void key(unsigned char key, int x, int y)
{
	if (key == '\033')
		exit(0);
}

const int TEXDIM = 256;

void processSpecialKeys(int key, int xx, int yy) {

	float fraction = 0.1f;
	cout << 'a';
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
	glutInit(&argc, argv);
	glutInitWindowSize(800,800);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL | GLUT_DOUBLE);
	(void)glutCreateWindow("Scena3D");
	glutDisplayFunc(renderScene);
	glutKeyboardFunc(key);
	glutSpecialFunc(processSpecialKeys);

	/* draw a perspective scene */
	glMatrixMode(GL_PROJECTION);
	glFrustum(-100., 100., -100., 100., 200., 1100.); // raportat la observator
	gluLookAt (x0, y0, z0, xref, yref, zref, Vx, Vy, Vz);
	/* turn on features and place light 0 in the right place */
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

	/* make display lists for sphere and cone; for efficiency */
	glNewList(LIGHT, GL_COMPILE);
	sphereLight = gluNewQuadric();
	gluSphere(sphereLight, 5.f, 20, 20);
	gluDeleteQuadric(sphereLight);
	glEndList();
	/* load pattern for current 2d texture */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	tex = make_texture(TEXDIM, TEXDIM);
	glTexImage2D(GL_TEXTURE_2D, 0, 1, TEXDIM, TEXDIM, 0, GL_RED, GL_FLOAT, tex);
	free(tex);

	glutMainLoop();

	return 0;
}
