#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
using namespace std;

const int TEXDIM = 256;
/* material properties for objects in scene */
static GLfloat wall_mat[] = { 0.87f, 0.63f, 0.07f, 1.f };
static GLfloat roof_mat[] = { 0.74f, 0.16f, 0.f, 1.f };
static GLfloat map_mat[] = { 0.f, 1.f, 0.3f, 1.f };
static GLfloat cone_mat[] = { 1.f, 0.4f, 0.f, 1.f };

GLuint qlist;

//Observer Coords
GLfloat eyex0 = 50.0, eyey0 = 450.0, eyez0 = 300.0; // X Y Z position of the camera
GLfloat xref = 50.0, yref = 150.0, zref = 300.0;
GLfloat Vx = 0.0, Vy = 1.0, Vz = 0.0;

float angle = 0.0; // angle of rotation for the camera direction
float lx = 1.0f, lz = -1.0f; // actual vector representing the camera's direction

// TODO with shadows and illumination
GLfloat Afara_xref = 50.0, Afara_yref = 150.0, Afara_zref = 300.0;
GLfloat Incasa_xref = 116.836 , Incasa_yref = 150.0 , Incasa_zref = -50.5317;

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

GLUquadricObj *sphereLight, *cone, *base, *houseLight, *qobj;

GLfloat plane[4];
GLfloat v0[3], v1[3], v2[3];


GLfloat lightpos[] = { -150.f, 350.f, -100.f, 1.f };
GLfloat houselightpos[] = { 180.f, 200.f, -400.f, 1.f };

GLfloat leftwallshadow[4][4];
GLfloat floorshadow[4][4];
static GLint fogMode;

GLfloat ctrlpoints[4][4][3] = {
   {{-60, -60, 160}, {-20, -60, 80},
    {20, -60, -40}, {60, -60, 80}},
   {{-60, -20, 40}, {-20, -20, 120},
    {20, -20, 0}, {60, -20, -40}},
   {{-60, 20, 160}, {-20, 20, 0},
    {20, 20, 120}, {60, 20, 280}},
   {{-60, 60, -80}, {-20, 60, -80},
    {20, 60, 0}, {60, 60, -40}}
};

enum {
	A, B, C, D
};

enum {
	X, Y, Z, W
};

enum {
	SPHERE = 1, LIGHT, CONE
};

enum {
  NONE, AFARA, INCASA
};

int rendermode = NONE;

enum {
	SHADOW, NOSHADOW
};

int rendershadowmode = SHADOW;

void menu(int selection) {
  rendermode = selection;
  glutPostRedisplay();
}

void con(void){
  glPushMatrix();
  glTranslatef(-50.f, 100.f, -400.f);
  glCallList(CONE);
  glPopMatrix();
}

void cilindru(void){
    glPushMatrix();
    glShadeModel (GL_SMOOTH);
    glTranslatef(-50.0, 0.0, -400.0);
    glPushMatrix();
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    glCallList(qlist);
    glPopMatrix();
	glFlush();
	glPopMatrix();
}
/* create a matrix that will project the desired shadow */
void shadowmatrix(GLfloat shadowMat[4][4],GLfloat groundplane[4],GLfloat lightpos[4]){
  GLfloat dot;

  /* find dot product between light position vector and ground plane normal */
  dot = groundplane[X] * lightpos[X] +
        groundplane[Y] * lightpos[Y] +
        groundplane[Z] * lightpos[Z] +
        groundplane[W] * lightpos[W];

  shadowMat[0][0] = dot - lightpos[X] * groundplane[X];
  shadowMat[1][0] = 0.f - lightpos[X] * groundplane[Y];
  shadowMat[2][0] = 0.f - lightpos[X] * groundplane[Z];
  shadowMat[3][0] = 0.f - lightpos[X] * groundplane[W];

  shadowMat[X][1] = 0.f - lightpos[Y] * groundplane[X];
  shadowMat[1][1] = dot - lightpos[Y] * groundplane[Y];
  shadowMat[2][1] = 0.f - lightpos[Y] * groundplane[Z];
  shadowMat[3][1] = 0.f - lightpos[Y] * groundplane[W];

  shadowMat[X][2] = 0.f - lightpos[Z] * groundplane[X];
  shadowMat[1][2] = 0.f - lightpos[Z] * groundplane[Y];
  shadowMat[2][2] = dot - lightpos[Z] * groundplane[Z];
  shadowMat[3][2] = 0.f - lightpos[Z] * groundplane[W];

  shadowMat[X][3] = 0.f - lightpos[W] * groundplane[X];
  shadowMat[1][3] = 0.f - lightpos[W] * groundplane[Y];
  shadowMat[2][3] = 0.f - lightpos[W] * groundplane[Z];
  shadowMat[3][3] = dot - lightpos[W] * groundplane[W];
}

/* find the plane equation given 3 points */
void findplane(GLfloat plane[4], GLfloat v0[3], GLfloat v1[3], GLfloat v2[3]) {
  GLfloat vec0[3], vec1[3];

  /* need 2 vectors to find cross product */
  vec0[X] = v1[X] - v0[X];
  vec0[Y] = v1[Y] - v0[Y];
  vec0[Z] = v1[Z] - v0[Z];

  vec1[X] = v2[X] - v0[X];
  vec1[Y] = v2[Y] - v0[Y];
  vec1[Z] = v2[Z] - v0[Z];

  /* find cross product to get A, B, and C of plane equation */
  plane[A] = vec0[Y] * vec1[Z] - vec0[Z] * vec1[Y];
  plane[B] = -(vec0[X] * vec1[Z] - vec0[Z] * vec1[X]);
  plane[C] = vec0[X] * vec1[Y] - vec0[Y] * vec1[X];

  plane[D] = -(plane[A] * v0[X] + plane[B] * v0[Y] + plane[C] * v0[Z]);
}

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
	if (rendershadowmode == SHADOW) {
	    glDisable(GL_DEPTH_TEST);
	    glDisable(GL_LIGHTING);
	    glColor3f(0.f, 0.f, 0.f);  /* shadow color */
	    glPushMatrix();
	    glMultMatrixf((GLfloat *) floorshadow);
	    con();
	    glPopMatrix();
	    glEnable(GL_DEPTH_TEST);
	    glEnable(GL_LIGHTING);
  	}
  	if (rendershadowmode == SHADOW) {
	    glEnable(GL_STENCIL_TEST);
	    glStencilFunc(GL_ALWAYS, 1, 0);
	    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	}

	/* walls */

	glBegin(GL_QUADS);
	/* left wall */
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(-100.f, 0.f, -200.f);
	glVertex3f(-100.f, 0.f, -500.f);
	glVertex3f(-100.f, 300.f, -500.f);
	glVertex3f(-100.f, 300.f, -200.f);
	glEnd();
	if (rendershadowmode == SHADOW) {
	    glStencilFunc(GL_EQUAL, 1, 1);
	    glDisable(GL_DEPTH_TEST);
	    glDisable(GL_LIGHTING);
	    glColor3f(0.f, 0.f, 0.f);  /* shadow color */
	    glDisable(GL_DEPTH_TEST);
	    glPushMatrix();
	    glMultMatrixf((GLfloat *) leftwallshadow);
	    con();
	    glPopMatrix();
	    glEnable(GL_DEPTH_TEST);
	    glDisable(GL_STENCIL_TEST);
	    glEnable(GL_DEPTH_TEST);
	    glEnable(GL_LIGHTING);
    }
    /* right wall & window*/
	glBegin(GL_QUADS);
	glNormal3f(-1.f, 0.f, 0.f);
	glVertex3f(200.f, 0.f, -200.f);
	glVertex3f(200.f, 0.f, -300.f);
	glVertex3f(200.f, 300.f, -300.f);
	glVertex3f(200.f, 300.f, -200.f);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(-1.f, 0.f, 0.f);
    glVertex3f(200.f, 0.f, -300.f);
    glVertex3f(200.f, 0.f, -400.f);
    glVertex3f(200.f, 100.f, -400.f);
    glVertex3f(200.f, 100.f, -300.f);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(-1.f, 0.f, 0.f);
    glVertex3f(200.f, 200.f, -300.f);
    glVertex3f(200.f, 200.f, -400.f);
    glVertex3f(200.f, 300.f, -400.f);
    glVertex3f(200.f, 300.f, -300.f);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(-1.f, 0.f, 0.f);
    glVertex3f(200.f, 0.f, -400.f);
    glVertex3f(200.f, 0.f, -500.f);
    glVertex3f(200.f, 300.f, -500.f);
    glVertex3f(200.f, 300.f, -400.f);
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
	glVertex3f(0.f, 250.f, -200.f);
	glVertex3f(100.f, 250.f, -200.f);
	glVertex3f(100.f, 300.f, -200.f);
	glVertex3f(0.f, 300.f, -200.f);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(100.f, 0.f, -200.f);
	glVertex3f(200.f, 0.f, -200.f);
	glVertex3f(200.f, 300.f, -200.f);
	glVertex3f(100.f, 300.f, -200.f);
	glEnd();

	// amestec albastru cu usa
	static GLfloat door_mat[] = { 0.0, 0.8, 0.8, 0.6 };
	glEnable (GL_BLEND);
	glDepthMask (GL_FALSE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, door_mat);
	glBegin(GL_QUADS);
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.f, -200.f);
	glVertex3f(0.f, 250.f, -200.f);
	glVertex3f(100.f, 250.f, -200.f);
	glVertex3f(100.f, 0.f, -200.f);
	glEnd();
	glDepthMask (GL_TRUE);
	glDisable (GL_BLEND);

    // amestec albastru cu geam
    static GLfloat window_mat[] = { 0.0, 0.8, 1.0, 0.7 };
    glEnable (GL_BLEND);
    glDepthMask (GL_FALSE);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, window_mat);
    glBegin(GL_QUADS);
    glNormal3f(0.f, 0.f, 1.f);
    glVertex3f(200.f, 100.f, -300.f);
    glVertex3f(200.f, 100.f, -400.f);
    glVertex3f(200.f, 200.f, -400.f);
    glVertex3f(200.f, 200.f, -300.f);
    glEnd();
    glDepthMask (GL_TRUE);
    glDisable (GL_BLEND);

}

void build_roof(){
	/* Roof */
	//Left
	glBegin(GL_QUADS);
	glNormal3f(1.f, 0.f, 0.f);
	glVertex3f(-100.f, 300.f, -200.f);
	glVertex3f(-100.f, 300.f, -500.f);
	glVertex3f(50.f, 500.f, -500.f);
	glVertex3f(50.f, 500.f, -200.f);
	glEnd();
	//Right
	glBegin(GL_QUADS);
	glNormal3f(-1.f, 0.f, 0.f);
	glVertex3f(200.f, 300.f, -200.f);
	glVertex3f(200.f, 300.f, -500.f);
	glVertex3f(50.f, 500.f, -500.f);
	glVertex3f(50.f, 500.f, -200.f);
	glEnd();
	//Front
	glBegin(GL_TRIANGLES);
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(-100.f, 300.f, -200.f);
	glVertex3f(50.f, 500.f, -200.f);
	glVertex3f(200.f, 300.f, -200.f);
	glEnd();
	//Back
	glBegin(GL_TRIANGLES);
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(-100.f, 300.f, -500.f);
	glVertex3f(50.f, 500.f, -500.f);
	glVertex3f(200.f, 300.f, -500.f);
	glEnd();
}

void renderScene(void){
    int i, j;

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	// Reset transformations
	glLoadIdentity();

    switch (rendermode) {
	    case NONE:
	    	// Add the new transformation
			//glFrustum(-100., 100., -100., 100., 200., 1100.); // raportat la observator
	    	gluPerspective(45.f,1.f,200.f,1000.f);
			gluLookAt(xref, yref, zref, xref+lx, yref, zref+lz, Vx, Vy, Vz); // reset camera
		break;
		case AFARA:
			cout << "afara";
			xref = Afara_xref;
			yref = Afara_yref;
			zref = Afara_zref;
			angle = 0.0;
			lx = 1.0f;
			lz = -1.0f;
			rendermode = NONE;
		break;
		case INCASA:
		    cout << "incasa";
            xref = Incasa_xref;
			yref = Incasa_yref;
			zref = Incasa_zref;
			angle = 0.0;
			lx = 1.0f;
			lz = -1.0f;
			rendermode = NONE;
		break;
    };

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, wall_mat);
	build_house();

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, map_mat);
	build_map();

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, roof_mat);
	build_roof();

 	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cone_mat);
    con();

    cilindru();

	//add sphereLight
	glPushMatrix();
	glTranslatef(lightpos[X], lightpos[Y], lightpos[Z]);
	glDisable(GL_LIGHTING);
	glColor3f(1.f, 1.f, .7f);
	glCallList(LIGHT);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(houselightpos[X], houselightpos[Y], houselightpos[Z]);
	glDisable(GL_LIGHTING);
	glColor3f(1.f, 1.f, .7f);
	glCallList(LIGHT);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(50,300,-150);
    glPushMatrix();
    glRotatef(180.0, 0.0,0.0,1.0);
    glEvalMesh2(GL_FILL, 0, 20, 0, 20);
    glPopMatrix();
    glPopMatrix();

	glutSwapBuffers();
}

void key(unsigned char key, int x, int y){
	if (key == '\033')
		exit(0);

    if (key == 'f')
    {
        glEnable(GL_FOG);
        {
           GLfloat fogColor[4] = {0.5f, 0.5f, 0.5f, 1.0f};
           fogMode = GL_EXP; //GL_EXP, GL_EXP2, GL_LINEAR
           glFogi(GL_FOG_MODE, fogMode);
           glFogfv(GL_FOG_COLOR, fogColor);
           glFogf(GL_FOG_DENSITY, 0.005f);
           glHint(GL_FOG_HINT, GL_DONT_CARE);
           glFogf(GL_FOG_START, 200.0);
           glFogf(GL_FOG_END, -800.0);
        }
        glutPostRedisplay();
    }

    if (key == 'n')
    {
        glDisable(GL_FOG);
        glutPostRedisplay();
    }
}

void reshape(GLsizei w, GLsizei h){
	// TODO
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;

	float ratio =  w * 1.0 / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45,ratio,200,1000);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

void processSpecialKeys(int key, int xx, int yy) {
	float fraction = 10.0f;
	switch (key) {
		case GLUT_KEY_LEFT :
			angle -= 0.01f;
			lx = sin(angle);
			lz = -cos(angle);
			break;
		case GLUT_KEY_RIGHT :
			angle += 0.01f;
			lx = sin(angle);
			lz = -cos(angle);
			break;
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

void initialize(){
	/* draw a perspective scene */
	glMatrixMode(GL_PROJECTION);
	gluPerspective(45.f,1.f,200.f,1000.f);
	gluLookAt (xref, yref, zref, xref+lx, yref, zref+lz, Vx, Vy, Vz);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	/* make shadow matricies */

	/* 3 points on floor */

    v0[X] = -100.f;
	v0[Y] = 0.f;
	v0[Z] = -500.f;

	v1[X] = 200.f;
	v1[Y] = 0.f;
	v1[Z] = -500.f;

	v2[X] = 100.f;
	v2[Y] = 0.f;
	v2[Z] = -200.f;

	findplane(plane, v0, v1, v2);
	shadowmatrix(floorshadow, plane, houselightpos);

	/* 3 points on left wall */

    v0[X] = -100.f;
	v0[Y] = 300.f;
	v0[Z] = -500.f;

	v1[X] = -100.f;
	v1[Y] = 300.f;
	v1[Z] = -200.f;

	v2[X] = -100.f;
	v2[Y] = 0.f;
	v2[Z] = -500.f;

	findplane(plane, v0, v1, v2);
	shadowmatrix(leftwallshadow, plane, houselightpos);


	/* turn on features and place light 0 and 1 in the right place */
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	glLightfv(GL_LIGHT1, GL_POSITION, houselightpos);

	/* make display lists for sphere and cone; for efficiency */
	glNewList(LIGHT, GL_COMPILE);
	sphereLight = gluNewQuadric();
	gluSphere(sphereLight, 5.f, 20, 20);
	gluDeleteQuadric(sphereLight);
	glEndList();

	glNewList(CONE, GL_COMPILE);
	cone = gluNewQuadric();
	base = gluNewQuadric();
	glRotatef(-90.f, 1.f, 0.f, 0.f);
	gluDisk(base, 0., 20., 20, 1);
	gluCylinder(cone, 20., 0., 60., 20, 20);
	gluDeleteQuadric(cone);
	gluDeleteQuadric(base);
	glEndList();

    qlist = glGenLists(1);
    qobj = gluNewQuadric();
    gluQuadricDrawStyle(qobj, GLU_FILL); /* smooth shaded */
    gluQuadricOrientation (qobj, GLU_INSIDE);
    gluQuadricNormals(qobj, GLU_SMOOTH);
    glNewList(qlist, GL_COMPILE);
    gluCylinder(qobj, 20, 20, 100, 15, 5);
    glEndList();

	/* load pattern for current 2d texture */
	/*
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	tex = make_texture(TEXDIM, TEXDIM);
	glTexImage2D(GL_TEXTURE_2D, 0, 1, TEXDIM, TEXDIM, 0, GL_RED, GL_FLOAT, tex);
	free(tex);
	*/

	// Texture type, Level-of-detail, Internal Pixel Format, Width, Height , 0 , RGB, FLOAT, pixels array
	float pixels[] = {
	    1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f
	};
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels);

    glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4,
            0, 1, 12, 4, &ctrlpoints[0][0][0]);

    glEnable(GL_MAP2_VERTEX_3);

    glMapGrid2f(20, 0.0, 1.0, 20, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);
}

int main(int argc, char *argv[]){
	glutInit(&argc, argv);
	glutInitWindowSize(600,600);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL | GLUT_DOUBLE);
	glutCreateWindow("Scena3D");
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	//glutReshapeFunc(reshape);
	glutKeyboardFunc(key);
	glutSpecialFunc(processSpecialKeys); // ZREF not working as expected - we must move in depth

	glutCreateMenu(menu);
    glutAddMenuEntry("Afara", AFARA);
    glutAddMenuEntry("In Casa", INCASA);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
	glEnable(GL_DEPTH_TEST);
	initialize();
	glutMainLoop();
	return 0;
}
