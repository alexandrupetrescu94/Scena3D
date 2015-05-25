#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
using namespace std;

const int TEXDIM = 256;

//Observer Coords
GLfloat eyex0 = 50.0, eyey0 = 450.0, eyez0 = 300.0; // X Y Z position of the camera
GLfloat xref = 50.0, yref = 150.0, zref = 300.0;
GLfloat Vx = 0.0, Vy = 1.0, Vz = 0.0;

float angle = 0.0; // angle of rotation for the camera direction
float lx = 1.0f, lz = -1.0f; // actual vector representing the camera's direction

// TODO with shadows and illumination
GLfloat Afara_xref = 50.0, Afara_yref = 150.0, Afara_zref = 300.0;
GLfloat Incasa_xref = -150.0 , Incasa_yref = 150.0 , Incasa_zref = -350.0;

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

GLUquadricObj *sphereLight, *cone, *base;

GLfloat lightpos[] = { -150.f, 350.f, -100.f, 1.f };

GLfloat leftwallshadow[4][4];
GLfloat floorshadow[4][4];
static GLint fogMode;

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
}

void renderScene(void){
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	// Reset transformations
	glLoadIdentity();

    switch (rendermode) {
	    case NONE:
	    	// Add the new transformation
			glFrustum(-100., 100., -100., 100., 200., 1100.); // raportat la observator
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

	/* material properties for objects in scene */
	static GLfloat wall_mat[] = { 1.f, 1.f, 1.f, 1.f };
	static GLfloat map_mat[] = { 0.f, 1.f, 0.3f, 1.f };
    static GLfloat cone_mat[] = { 1.f, 0.4f, 0.f, 1.f };

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, wall_mat);
	build_house();

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, map_mat);
	build_map();

	//glDisable(GL_DEPTH_TEST);
    //glDisable(GL_LIGHTING);
    //glColor3f(0.f, 0.f, 0.f);  /* shadow color */
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cone_mat);
    //glMultMatrixf((GLfloat *) floorshadow);
    con();
    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_LIGHTING);

	//add sphereLight
	glPushMatrix();
	glTranslatef(lightpos[X], lightpos[Y], lightpos[Z]);
	glDisable(GL_LIGHTING);
	glColor3f(1.f, 1.f, .7f);
	glCallList(LIGHT);
	glEnable(GL_LIGHTING);
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
	gluPerspective(45,ratio,1,100);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

void processSpecialKeys(int key, int xx, int yy) {
	float fraction = 2.0f;
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
	glFrustum(-100., 100., -100., 100., 200., 1100.); // raportat la observator
	gluLookAt (xref, yref, zref, xref, yref, zref, Vx, Vy, Vz);

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

	glNewList(CONE, GL_COMPILE);
	cone = gluNewQuadric();
	base = gluNewQuadric();
	glRotatef(-90.f, 1.f, 0.f, 0.f);
	gluDisk(base, 0., 20., 20, 1);
	gluCylinder(cone, 20., 0., 60., 20, 20);
	gluDeleteQuadric(cone);
	gluDeleteQuadric(base);
	glEndList();


	/* load pattern for current 2d texture */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	tex = make_texture(TEXDIM, TEXDIM);
	glTexImage2D(GL_TEXTURE_2D, 0, 1, TEXDIM, TEXDIM, 0, GL_RED, GL_FLOAT, tex);
	free(tex);
}

int main(int argc, char *argv[]){
  	GLfloat plane[4];
  	GLfloat v0[3], v1[3], v2[3];

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
	initialize();
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
	return 0;
}
