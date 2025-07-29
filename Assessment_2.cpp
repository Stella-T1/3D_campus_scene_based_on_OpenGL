#define FREEGLUT_STATIC
#include<GL/freeglut.h>
#include <vector>
#define M_PI 3.1415927

using namespace std;

GLint imagewidth[6], imageheight[6];
GLint pixellength[6];
vector<GLubyte*> p;
GLuint texture[6];

int intWinWidth = 800; // Default window size
int intWinHeight = 600;

// Cameras and Viewpoints
float fltX0 = 0.0, fltY0 = 180.0, fltZ0 = 75.0; // Camera position
float fltXRef = 0.0, fltYRef = 0.0, fltZRef = 0.0; // Look At reference point
float angleX0 = 0.0, angleY0 = 0.0; // Angle of view rotation
float moveSpeed = 1.5; // The speed at which the camera moves
// Mouse interaction
float lastMouseX = -1, lastMouseY = -1; // Record the last mouse position
float sensitivity = 0.1f; // Mouse sensitivity
float initialDistance = 100.0f; // The initial distance between the camera and the observed target
float currentDistance = initialDistance; // The distance between the current camera and the observed target

// Set up lightings
typedef enum {
    LIGHT_MORNING,
    LIGHT_AFTERNOON,
    LIGHT_NIGHT
} LightState;

LightState currentState = LIGHT_MORNING; // Initial state

// Set up materials
// Road
GLfloat ambient_road[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat diffuse_road[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat specular_road[] = { 0.3f, 0.3f, 0.3f, 1.0f };
GLfloat shine_road[] = { 25.0f };
// Tree
GLfloat ambient_tree[] = { 0.0f, 0.7f, 0.1f, 1.0f };
GLfloat diffuse_tree[] = { 0.0f, 0.7f, 0.1f, 1.0f };
GLfloat ambient_trunk[] = { 0.7f, 0.3f, 0.0f, 1.0f };
GLfloat diffuse_trunk[] = { 0.7f, 0.3f, 0.0f, 1.0f };
// Bush
GLfloat ambient_bush[] = { 0.5f, 0.7f, 0.3f, 1.0f };
GLfloat diffuse_bush[] = { 0.5f, 0.7f, 0.3f, 1.0f };
// Car
GLfloat ambient_car1[] = { 0.0f, 0.0f, 0.5f, 1.0f };
GLfloat diffuse_car1[] = { 0.0f, 0.0f, 0.5f, 1.0f };

GLfloat ambient_car2[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat diffuse_car2[] = { 1.0f, 1.0f, 1.0f, 1.0f };

GLfloat ambient_carbody[] = { 1.0f, 0.84f, 0.0f, 1.0f };
GLfloat diffuse_carbody[] = { 1.0f, 0.84f, 0.0f, 1.0f };

GLfloat ambient_wheel[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat diffuse_wheel[] = { 0.0f, 0.0f, 0.0f, 1.0f };
// Lawn
GLfloat ambient_lawn[] = { 0.7f, 0.6f, 0.63f, 1.0f };
GLfloat diffuse_lawn[] = { 0.7f, 0.6f, 0.63f, 1.0f };

// Flag
GLfloat x0 = 0.0f; // Flag vertex data
GLfloat length = 15.0f;
GLfloat flagPhase = 0.0f; // Wave phase of the flag

// Car
struct Car {
    GLfloat x;
    GLfloat speed;
}car1 = { 40.0f, 0.15f }, car2 = { 40.0f, 0.2f }; // The position and speed of the vehicle
int startTime = 0;

GLfloat wheelAngle1 = 0.0f; // Wheel rotation Angle
GLfloat wheelAngle2 = 0.0f;
GLfloat wheelAngle3 = 0.0f;
GLfloat wheelAngle4 = 0.0f;


// Read texture files
void ReadImage(const char path[256], GLint& imagewidth, GLint& imageheight, GLint& pixellength)
{
    GLubyte* pixeldata;
    FILE* pfile;
    fopen_s(&pfile, path, "rb");
    if (pfile == 0) exit(0);

    fseek(pfile, 0x0012, SEEK_SET);
    fread(&imagewidth, sizeof(imagewidth), 1, pfile);
    fread(&imageheight, sizeof(imageheight), 1, pfile);

    pixellength = imagewidth * 3;
    while (pixellength % 4 != 0)pixellength++;
    pixellength *= imageheight;

    pixeldata = (GLubyte*)malloc(pixellength);
    if (pixeldata == 0) exit(0);

    fseek(pfile, 54, SEEK_SET);
    fread(pixeldata, pixellength, 1, pfile);
    p.push_back(pixeldata);
    fclose(pfile);
}

// Define material and light properties
void myTexture()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0f); // Set the depth buffer clear value to 1.0 (far plane)
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_TEXTURE_2D);
    const char* filenames[6] = { "sky.bmp", "road.bmp", "SB1.bmp", "SB2.bmp", "CB.bmp", "CBT.bmp" };
    for (int i = 0; i < 6; i++) {
        ReadImage(filenames[i], imagewidth[i], imageheight[i], pixellength[i]);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &texture[i]);
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, imagewidth[i], imageheight[i], 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, p[i]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    // Set GL_MODULATE so that the texture color is multiplied by the vertex color
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glDisable(GL_CULL_FACE);
}

void myLight()
{
    glEnable(GL_LIGHTING);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    // The color of the object is calculated from the material properties and lighting
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Environment light
    GLfloat light_morning[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    GLfloat light_afternoon[] = { 0.45f, 0.35f, 0.45f, 1.0f };
    GLfloat light_night[] = { 0.35f, 0.35f, 0.4f, 1.0f };

    // Set the ambient light according to the current state
    switch (currentState) {
    case LIGHT_MORNING:
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_morning);
        break;
    case LIGHT_AFTERNOON:
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_afternoon);
        break;
    case LIGHT_NIGHT:
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_night);
        break;
    }

    // Spot light
    GLfloat lightPosition0[] = { 0.0f, 100.0f, 100.0f, 1.0f };
    GLfloat lightAmbient0[] = { 0.8f, 0.8f, 0.1f, 1.0f };
    GLfloat lightDiffuse0[] = { 1.0f, 1.0f, 0.4f, 1.0f };
    GLfloat lightSpecular0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat lightDirection0[] = { 1.0f, -1.0f, -1.0f, 0.0f };

    GLfloat lightPosition1[] = { 0.0f, 120.0f, 100.0f, 1.0f };
    GLfloat lightAmbient1[] = { 0.9f, 0.5f, 0.4f, 1.0f };
    GLfloat lightDiffuse1[] = { 0.9f, 0.2f, 0.2f, 1.0f };
    GLfloat lightSpecular1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat lightDirection1[] = { -1.0f, -1.0f, 1.0f, 0.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular0);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDirection0);
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 50);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.7);

    glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular1);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, lightDirection1);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 10);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 10);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0);

    // Parallel light
    GLfloat lightPosition2[] = { 100.0f, 120.0f, -1.0f, 0.0f };
    GLfloat lightAmbient2[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat lightDiffuse2[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat lightSpecular2[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glLightfv(GL_LIGHT2, GL_POSITION, lightPosition2);
    glLightfv(GL_LIGHT2, GL_AMBIENT, lightAmbient2);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, lightDiffuse2);
    glLightfv(GL_LIGHT2, GL_SPECULAR, lightSpecular2);

    switch (currentState) {
    case LIGHT_NIGHT:
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHT1);
        glDisable(GL_LIGHT2);
        break;
    case LIGHT_AFTERNOON:
        glDisable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        glDisable(GL_LIGHT2);
        break;
    case LIGHT_MORNING:
        glEnable(GL_LIGHT0);
        glDisable(GL_LIGHT1);
        glEnable(GL_LIGHT2);
        break;
    }
}

//  Idle callback function
void when_in_mainloop()
{
    glutPostRedisplay();
}

// Animations
// Simulate the wheel rotation of a car to drive the car forward
void carTimer(int value) {
    car1.x += car1.speed;
    if (car1.x > 168) car1.x = 40; // Reset location
    wheelAngle1 += 2.0f; // Update wheel rotation Angle
    wheelAngle2 += 2.0f;
    wheelAngle3 += 2.0f;
    wheelAngle4 += 2.0f;

    car2.x += car2.speed;
    if (car2.x > 168) car2.x = 40;
    wheelAngle1 += 3.0f;
    wheelAngle2 += 3.0f;
    wheelAngle3 += 3.0f;
    wheelAngle4 += 3.0f;

    glutPostRedisplay();
    glutTimerFunc(16, carTimer, 1);
}
// Simulate the wave of the flag
void flagTimer(int value) {
    flagPhase += 0.45; // Update wave phase
    glutTimerFunc(16, flagTimer, 0);
}

// Draw objects
// Draw the skybox, simulate the scene of the sky and ground
void drawSkybox()
{
    glPushMatrix();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    // Front face
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glColor3f(0.7f, 0.8f, 1.0f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-100.0f, 200.0f, 100.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(100.0f, 200.0f, 100.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(100.0f, 0.0f, 100.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-100.0f, 0.0f, 100.0f);
    glEnd();

    // Behind face
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glColor3f(0.7f, 0.8f, 1.0f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-100.0f, 200.0f, -100.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-100.0f, 0.0f, -100.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(100.0f, 0.0f, -100.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(100.0f, 200.0f, -100.0f);
    glEnd();

    // Left face
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glColor3f(0.7f, 0.8f, 1.0f);
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-100.0f, 200.0f, -100.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-100.0f, 0.0f, -100.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-100.0f, 0.0f, 100.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-100.0f, 200.0f, 100.0f);
    glEnd();

    // Right face
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glColor3f(0.7f, 0.8f, 1.0f);
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(100.0f, 200.0f, -100.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(100.0f, 0.0f, -100.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(100.0f, 0.0f, 100.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(100.0f, 200.0f, 100.0f);
    glEnd();

    // Top face
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glColor3f(0.7f, 0.8f, 1.0f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-100.0f, 200.0f, -100.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(100.0f, 200.0f, -100.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(100.0f, 200.0f, 100.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-100.0f, 200.0f, 100.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    // Bottom face
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-100.0f, 0.0f, -100.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(100.0f, 0.0f, -100.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(100.0f, 0.0f, 100.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-100.0f, 0.0f, 100.0f);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

// Bezier curve
void drawBezierCurve() {
    // Define the control points of the cubic Bezier curve
    GLfloat p0[] = { -70.0f, 0.0f, 0.0f };
    GLfloat p1[] = { -2.0f, 0.0f, 32.0f };
    GLfloat p2[] = { 2.0f, 0.0f, -32.0f };
    GLfloat p3[] = { 68.0f, 0.0f, 0.0f };

    /*
    To draw a Bezier curve using a line segment approximation:
    1. First, loop over the value of the parameter t from 0 to 1 in steps of 1/100.
    2. For each value of t, calculate the coordinates of the points on the corresponding curve according to the formula of the Bezier curve.
    3. The calculated points are then plotted using glVertex3f to form an approximate Bezier curve.
    */
    glBegin(GL_LINE_STRIP);
    glLineWidth(5.0f);
    for (int i = 0; i <= 100; ++i) {
        GLfloat t = static_cast<GLfloat>(i) / 100.0f;
        GLfloat u = 1.0f - t;
        GLfloat tt = t * t;
        GLfloat uu = u * u;
        GLfloat uuu = uu * u;
        GLfloat ttt = tt * t;

        GLfloat x = uuu * p0[0] + 3 * uu * t * p1[0] + 3 * u * tt * p2[0] + ttt * p3[0];
        GLfloat y = uuu * p0[1] + 3 * uu * t * p1[1] + 3 * u * tt * p2[1] + ttt * p3[1];
        GLfloat z = uuu * p0[2] + 3 * uu * t * p1[2] + 3 * u * tt * p2[2] + ttt * p3[2];

        glVertex3f(x, y, z);
    }
    glEnd();
}

// CB
void drawTranTriangularPrism()
{
    glPushMatrix();
    // Base of the front triangle
    glBegin(GL_TRIANGLES);
    glVertex3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glEnd();

    // Bottom of the back triangle
    glBegin(GL_TRIANGLES);
    glVertex3f(-1.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 1.0f, 1.0f);
    glEnd();

    // Side 1
    glBegin(GL_QUADS);
    glVertex3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f, 0.0f, 1.0f);
    glEnd();

    // Side 2
    glBegin(GL_QUADS);
    glVertex3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 0.0f, 1.0f);
    glEnd();

    // Side 3
    glBegin(GL_QUADS);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glEnd();

    glPopMatrix();
}

void drawBase()
{
    // Left
    glPushMatrix();
    glColor3f(0.7f, 0.7f, 0.7f);
    glTranslatef(0.0f, 4.0f, -60.0f);
    glScalef(0.5f, 0.4f, 1.2f);
    glutSolidCube(20.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.8f, 0.8f, 0.8f);
    glTranslatef(-5.0f, 0.0f, -48.0f);
    glScalef(10.0f, 8.0f, 9.5f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    drawTranTriangularPrism();
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.8f, 0.8f, 0.8f);
    glTranslatef(-4.5f, 0.0f, -65.0f);
    glScalef(6.5f, 6.0f, 9.5f);
    drawTranTriangularPrism();
    glPopMatrix();

    // Center
    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    glTranslatef(15.0f, 4.0f, -60.0f);
    glScalef(1.0f, 0.4f, 1.0f);
    glutSolidCube(20.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    glTranslatef(10.0f, 0.0f, -50.0f);
    glScalef(9.0f, 6.5f, 7.5f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    drawTranTriangularPrism();
    glPopMatrix();

    // Right
    glPushMatrix();
    glColor3f(0.7f, 0.7f, 0.7f);
    glTranslatef(30.0f, 4.0f, -60.0f);
    glScalef(0.5f, 0.4f, 1.2f);
    glutSolidCube(20.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.8f, 0.8f, 0.8f);
    glTranslatef(25.0f, 0.0f, -48.0f);
    glScalef(10.0f, 8.0f, 9.5f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    drawTranTriangularPrism();
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.8f, 0.8f, 0.8f);
    glTranslatef(35.0f, 0.0f, -55.0f);
    glScalef(6.5f, 6.0f, 9.5f);
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    drawTranTriangularPrism();
    glPopMatrix();
}

void drawCenterbuilding()
{
    // Base
    glPushMatrix();
    glTranslatef(-20.0f, 0.0f, 20.0f);
    glScalef(1.6f, 1.6f, 1.6f);
    drawBase();
    glPopMatrix();

    // Center
    glPushMatrix();
    glTranslatef(4.0f, 33.0f, -76.0f);
    glScalef(8.0f, 8.0f, 8.0f);

    // Front
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, -0.5f, 2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, -2.5f, 2.0f);
    glTexCoord2f(1.0f, 0.65f); glVertex3f(2.0f, -2.5f, 2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(2.0f, -1.4f, 2.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, 1.8f, 2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, -0.2f, 2.0f);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(-0.3f, -0.2f, 2.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(-0.3f, 1.8f, 2.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(2.0f, 1.8f, 2.0f);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(2.0f, -1.0f, 2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.3f, -0.2f, 2.0f);
    glTexCoord2f(0.0f, 0.85f); glVertex3f(0.3f, 1.8f, 2.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Right
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.5f); glVertex3f(2.0f, -1.4f, 2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.0f, -2.5f, 2.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(2.0f, -2.5f, -2.0f);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(2.0f, -0.6f, -2.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(2.0f, 1.8f, 2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.0f, -1.0f, 2.0f);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(2.0f, -0.5f, 0.1f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(2.0f, 1.8f, 0.6f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.0f, -0.5f, 0.1f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(2.0f, 1.2f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(2.0f, 1.2f, 0.9f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(2.0f, 1.8f, -0.1f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(2.0f, 1.8f, -2.0f);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(2.0f, -0.2f, -2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.0f, -0.2f, -0.1f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Left
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-2.0f, -0.5f, 2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.0f, -2.5f, 2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, -2.5f, -2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, -0.2f, -2.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(-2.0f, 1.8f, 2.0f);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(-2.0f, -0.2f, 2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, 0.3f, 0.1f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, 1.8f, 0.6f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, 0.3f, 0.1f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, 1.2f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-2.0f, 1.2f, 0.9f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(-2.0f, 1.8f, -0.1f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, 1.8f, -2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, 0.5f, -2.0f);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(-2.0f, 0.5f, -0.1f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Behind
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(2.0f, -0.6f, -2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.0f, -2.5f, -2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-2.0f, -2.5f, -2.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-2.0f, -0.2f, -2.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, 0.5f, -2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, 1.8f, -2.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(-0.3f, 1.8f, -2.0f);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(-0.3f, 0.1f, -2.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(2.0f, 1.8f, -2.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(2.0f, -0.2f, -2.0f);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(0.3f, 0.1f, -2.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.3f, 1.8f, -2.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Top
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[5]);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, 1.8f, 2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f, 1.8f, -2.0f);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(2.0f, 1.8f, -2.0f);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(2.0f, 1.8f, 2.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    // Solid
    glPushMatrix();
    glColor3f(0.7f, 0.8f, 0.8f);
    glTranslatef(4.0f, 34.5f, -76.0f);
    glScalef(1.05f, 0.85f, 1.0f);
    glutSolidCube(30.0f);
    glPopMatrix();
}

// Flag
void Flag(GLfloat r1, GLfloat g1, GLfloat b1, GLfloat r2, GLfloat g2, GLfloat b2)
{
    glPushMatrix();
    glTranslatef(60.0f, 12.0f, -75.0f);
    glScalef(1.2f, 26.0f, 1.2f);
    glColor3f(0.7f, 0.7f, 0.7f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(60.0f, 25.5f, -75.0f);
    glScalef(0.8f, 0.8f, 0.8f);
    glColor3f(1.0f, 1.0f, 1.0f);
    gluSphere(gluNewQuadric(), 1.0f, 50.0f, 50.0f);
    glPopMatrix();

    // Draw the flag
    glPushMatrix();
    glTranslatef(60.0f, 20.0f, -75.0f);
    glScalef(0.5f, 1.0f, 0.5f); // Resizing flag

    glBegin(GL_QUAD_STRIP);
    for (GLfloat i = x0; i <= x0 + length; i = i + 1) {
        GLfloat y1 = 2.0f * sin((i + flagPhase) / (8.5f));

        GLfloat t = (y1 + 3) / 3.0f; // Interpolation parameters [0, 1]
        glColor3f(r1 + t * (r2 - r1), g1 + t * (g2 - g1), b1 + t * (b2 - b1));

        glVertex2f(i, y1 - 3);
        glVertex2f(i, y1 + 3);
    }
    glEnd();
    glPopMatrix();
}

void drawFlag()
{
    glPushMatrix();
    glTranslatef(0.0, 0.0, 0.0);
    Flag(0.16, 0.047, 0.35, 0.7, 0.0, 0.49);
    glPopMatrix();

    glPushMatrix();
    glScalef(1.0, 0.95, 1.0);
    glTranslatef(0.0, 0.0, -5.0);
    Flag(0.9, 0.54, 0.4, 0.7, 0.0, 0.49);
    glPopMatrix();

    glPushMatrix();
    glScalef(1.0, 0.85, 1.0);
    glTranslatef(0.0, 0.0, 5.0);
    Flag(0.96, 0.4, 0.27, 0.7, 0.0, 0.49);
    glPopMatrix();
}

// S buildings
// Draw the main building of S Building
void drawSCube()
{
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);

    // Front
    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glColor3f(0.7f, 0.7f, 0.7f);// Medium grey
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-12.0f, 0.0f, 5.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(5.0f, 0.0f, 5.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(5.0f, 30.0f, 5.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-12.0f, 30.0f, 5.0f);
    glEnd();

    // Behind
    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-12.0f, 0.0f, -35.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-12.0f, 30.0f, -35.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(5.0f, 30.0f, -35.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(5.0f, 0.0f, -35.0f);
    glEnd();

    // Left
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-12.0f, 0.0f, -35.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-12.0f, 30.0f, -35.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-12.0f, 30.0f, 5.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-12.0f, 0.0f, 5.0f);

    // Right
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(5.0f, 0.0f, -35.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(5.0f, 0.0f, 5.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(5.0f, 30.0f, 5.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(5.0f, 30.0f, -35.0f);
    glEnd();

    // Top
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glColor3f(0.8f, 0.8f, 0.8f); // Light grey
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-12.0f, 30.0f, -35.0f);
    glTexCoord2f(0.02f, 0.0f); glVertex3f(5.0f, 30.0f, -35.0f);
    glTexCoord2f(0.02f, 0.02f); glVertex3f(5.0f, 30.0f, 5.0f);
    glTexCoord2f(0.0f, 0.02f); glVertex3f(-12.0f, 30.0f, 5.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // Bottom
    glColor3f(0.5f, 0.5f, 0.5f); // Dark grey
    glBegin(GL_QUADS);
    glVertex3f(-12.0f, 0.0f, -35.0f);
    glVertex3f(5.0f, 0.0f, -35.0f);
    glVertex3f(5.0f, 0.0f, 5.0f);
    glVertex3f(-12.0f, 0.0f, 5.0f);
    glEnd();
}

void drawScienceBuilding()
{
    // SA
    glPushMatrix();
    glTranslatef(70.0, 0.0f, 55.0);
    glScalef(1.2f, 1.2f, 1.2f);
    drawSCube();
    glPopMatrix();
    // SB
    glPushMatrix();
    glTranslatef(30.0, 0.0f, 55.0);
    glScalef(1.2f, 1.2f, 1.2f);
    drawSCube();
    glPopMatrix();
    // SC
    glPushMatrix();
    glTranslatef(-10.0, 0.0f, 55.0);
    glScalef(1.2f, 1.2f, 1.2f);
    drawSCube();
    glPopMatrix();
    // SD
    glPushMatrix();
    glTranslatef(-50.0, 0.0f, 55.0);
    glScalef(1.2f, 1.2f, 1.2f);
    drawSCube();
    glPopMatrix();

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    // Connecting channel
    glPushMatrix();
    glColor4f(0.7f, 0.8f, 1.0f, 0.5f);
    glTranslatef(45.0, 16.0f, 25.0);
    glScalef(0.55, 0.4f, 0.3f);
    glutSolidCube(35.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(5.5, 16.0f, 25.0);
    glScalef(0.55, 0.4f, 0.3f);
    glutSolidCube(35.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-34.5, 16.0f, 25.0);
    glScalef(0.55, 0.4f, 0.3f);
    glutSolidCube(35.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(45.0, 18.0f, 50.0);
    glScalef(0.55, 0.3f, 0.3f);
    glutSolidCube(35.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(5.5, 18.0f, 50.0);
    glScalef(0.55, 0.3f, 0.3f);
    glutSolidCube(35.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-34.5, 18.0f, 50.0);
    glScalef(0.55, 0.3f, 0.3f);
    glutSolidCube(35.0);
    glPopMatrix();

    // Cylinder
    glPushMatrix();
    glTranslatef(50.0, 13.0f, 51.0);
    glScalef(1.3f, 0.85f, 1.0f);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glColor3f(0.8, 0.8, 0.8);
    glutSolidCylinder(2.0, 15.0, 50, 1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(42.0, 13.0f, 51.0);
    glScalef(1.3f, 0.85f, 1.0f);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glColor3f(0.8, 0.8, 0.8);
    glutSolidCylinder(2.0, 15.0, 50, 1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(10.0, 13.0f, 51.0);
    glScalef(1.3f, 0.85f, 1.0f);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glColor3f(0.8, 0.8, 0.8);
    glutSolidCylinder(2.0, 15.0, 50, 1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.0, 13.0f, 51.0);
    glScalef(1.3f, 0.85f, 1.0f);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glColor3f(0.8, 0.8, 0.8);
    glutSolidCylinder(2.0, 15.0, 50, 1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-30.0, 13.0f, 51.0);
    glScalef(1.3f, 0.85f, 1.0f);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glColor3f(0.8, 0.8, 0.8);
    glutSolidCylinder(2.0, 15.0, 50, 1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-38.0, 13.0f, 51.0);
    glScalef(1.3f, 0.85f, 1.0f);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glColor3f(0.8, 0.8, 0.8);
    glutSolidCylinder(2.0, 15.0, 50, 1);
    glPopMatrix();

    glDisable(GL_NORMALIZE);
    glDisable(GL_COLOR_MATERIAL);
}

// Lawn
void drawLawn()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_lawn);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_lawn);
    glMaterialf(GL_FRONT, GL_SHININESS, 50);

    glPushMatrix();
    glTranslatef(10.0, 4.5f, -14.5);
    glScalef(8.0f, 0.3f, 1.1f);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glColor4f(0.7, 0.6, 0.63, 0.5);
    glutSolidCylinder(10.0, 15.0, 200, 1);
    glPopMatrix();

    // Draw border
    glPushMatrix();
    glTranslatef(10.0, 0, -14.5);
    glScalef(8.0f, 0.3f, 1.1f);
    // Draw the top and bottom circles
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 200; i++) {
        float angle = 2.0 * M_PI * i / 200;
        float x = 10 * cos(angle);
        float z = 10 * sin(angle);
        glVertex3f(x, 0.0, z);
    }
    glEnd();

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 200; i++) {
        float angle = 2.0 * M_PI * i / 200;
        float x = 10 * cos(angle);
        float z = 10 * sin(angle);
        glVertex3f(x, 15, z);
    }
    glEnd();
    glPopMatrix();
}

// Tree
void drawTree()
{
    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_tree);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_tree);
    glMaterialf(GL_FRONT, GL_SHININESS, 50);

    glTranslatef(0.0, 10.0, 0.0);
    glScalef(6.0, 6.5, 6.0);
    glColor3f(0.0, 0.7, 0.1);
    glutSolidIcosahedron();
    glColor3f(0.0, 0.0, 0.0);
    glutWireIcosahedron();
    glPopMatrix();

    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_trunk);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_trunk);
    glMaterialf(GL_FRONT, GL_SHININESS, 50);

    glTranslatef(0.0, 2.5, 0.0);
    glScalef(1.5, 5.0, 1.5);
    glColor3f(0.7, 0.3, 0.0);
    glutSolidCube(1);
    glColor3f(0.0, 0.0, 0.0);
    glutWireCube(1);
    glPopMatrix();
}

// Bush
void drawBush()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_bush);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_bush);
    glMaterialf(GL_FRONT, GL_SHININESS, 50);

    glPushMatrix();
    glTranslatef(0.0, 1.2, 0.0);
    glScalef(1.8, 1.8, 1.8);
    glColor3f(0.5, 0.7, 0.3);
    glutSolidDodecahedron(); // Solid 16-hedrons
    glColor3f(0.3, 0.5, 0.2);
    glutWireDodecahedron();
    glPopMatrix();
}

// Hierarchical modeling (Car)
// Draw wheel
void drawWheel(GLfloat x, GLfloat y, GLfloat z, GLfloat angle)
{
    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_wheel);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_wheel);
    glMaterialf(GL_FRONT, GL_SHININESS, 50);

    glTranslatef(x, y, z);
    glRotatef(angle, 0.0, 0.0, 1.0); // Rotate around the z-axis
    glColor3f(0.0, 0.0, 0.0);
    glutSolidCylinder(1.0, 2, 80, 1);
    glPopMatrix();
}

// Draw the carbody
void drawCarBody(GLfloat x, GLfloat y, GLfloat z)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_car1);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_car1);
    glMaterialf(GL_FRONT, GL_SHININESS, 50);

    glTranslatef(-80, 2.0, 4.0);
    glScalef(1.2, 1.1, 1.9);
    glColor3f(0.0f, 0.0f, 0.5f);
    glutSolidCube(3.0);
    glPopMatrix();

    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_car2);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_car2);
    glMaterialf(GL_FRONT, GL_SHININESS, 50);

    glTranslatef(-81.5, 1.8, 1.0);
    glScalef(2.5, 1.8, 6.0);
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    drawTranTriangularPrism();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-78.5, 1.8, 7.0);
    glScalef(2.5, 1.8, 6.0);
    glRotatef(180, 0.0, 1.0, 0.0);
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    drawTranTriangularPrism();
    glPopMatrix();

    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_carbody);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_carbody);
    glMaterialf(GL_FRONT, GL_SHININESS, 50);

    glTranslatef(-80, 1.0, 4.0);
    glScalef(3.0, 0.6, 2.0);
    glColor3f(1.0f, 0.84f, 0.0f);
    glutSolidCube(3.0);
    glPopMatrix();
    glPopMatrix();
}

// Draw car
void drawCar(GLfloat x, GLfloat y, GLfloat z)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    drawCarBody(0.0, 0.0, 0.0);
    drawWheel(-83, 0.0, 5.1, wheelAngle1);
    drawWheel(-77, 0.0, 5.1, wheelAngle2);
    drawWheel(-83, 0.0, 0.9, wheelAngle3);
    drawWheel(-77, 0.0, 0.9, wheelAngle4);
    glPopMatrix();
}

// Road
void drawRcube()
{
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    // Front
    glVertex3f(1.0, 0.1, 1.0);
    glVertex3f(-1.0, 0.1, 1.0);
    glVertex3f(-1.0, 0.1, -1.0);
    glVertex3f(1.0, 0.1, -1.0);
    // Behind
    glVertex3f(1.0, -0.1, -1.0);
    glVertex3f(-1.0, -0.1, -1.0);
    glVertex3f(-1.0, -0.1, 1.0);
    glVertex3f(1.0, -0.1, 1.0);
    // Top
    glVertex3f(1.0, 0.1, 1.0);
    glVertex3f(-1.0, 0.1, 1.0);
    glVertex3f(-1.0, 0.1, -1.0);
    glVertex3f(1.0, 0.1, -1.0);
    // Bottom
    glVertex3f(1.0, -0.1, 1.0);
    glVertex3f(-1.0, -0.1, 1.0);
    glVertex3f(-1.0, -0.1, -1.0);
    glVertex3f(1.0, -0.1, -1.0);
    // Left
    glVertex3f(-1.0, 0.1, 1.0);
    glVertex3f(-1.0, 0.1, -1.0);
    glVertex3f(-1.0, -0.1, -1.0);
    glVertex3f(-1.0, -0.1, 1.0);
    // Right
    glVertex3f(1.0, 0.1, -1.0);
    glVertex3f(1.0, 0.1, 1.0);
    glVertex3f(1.0, -0.1, 1.0);
    glVertex3f(1.0, -0.1, -1.0);
    glEnd();
}

void drawRoad()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_road);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_road);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular_road);
    glMaterialfv(GL_FRONT, GL_SHININESS, shine_road);

    // Main road
    glPushMatrix();
    glTranslatef(10, 0.0, 4.0);
    glScalef(75, 3.0, 8);
    drawRcube();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(10, 0.0, -33.0);
    glScalef(75, 3.0, 8);
    drawRcube();
    glPopMatrix();

    // Branch
    glPushMatrix();
    glTranslatef(46, 0.0, 45);
    glRotatef(90, 0, 1, 0);
    glScalef(35, 3.0, 8);
    drawRcube();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(5, 0.0, 45);
    glRotatef(90, 0, 1, 0);
    glScalef(35, 3.0, 8);
    drawRcube();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-35, 0.0, 45);
    glRotatef(90, 0, 1, 0);
    glScalef(35, 3.0, 8);
    drawRcube();
    glPopMatrix();
}

// Display all objects
void display()
{
    // Clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat)intWinWidth / (GLfloat)intWinHeight, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(fltX0, fltY0, fltZ0, fltXRef, fltYRef, fltZRef, 0.0, 1.0, 0.0);

    glTranslatef(fltX0, fltY0, fltZ0);
    glRotatef(angleX0, 1.0, 0.0, 0.0);
    glRotatef(angleY0, 0.0, 1.0, 0.0);
    glTranslatef(-fltX0, -fltY0, -fltZ0);

    // Draw objects
    glEnable(GL_COLOR_MATERIAL);
    drawSkybox();
    // CB
    drawCenterbuilding();
    // Draw flags
    drawFlag();
    // S Buidings
    drawScienceBuilding();
    // Lawn
    glPushMatrix();
    glTranslatef(10.0, 4.5f, -14.5);
    drawBezierCurve();
    glPopMatrix();
    drawLawn();
    // Road
    drawRoad();
    // Draw trees
    glPushMatrix();
    glTranslatef(-60, 4.0, -14.5);
    drawTree();
    for (int i = 0; i <= 8; i++) {
        glTranslatef(15.0, 0.0, 0.0);
        drawTree();
    }
    glPopMatrix();
    // Draw bush
    glPushMatrix();
    glTranslatef(-48, 4.0, -10);
    drawBush();
    for (int i = 0; i <= 10; i++) {
        glTranslatef(10.0, 0.0, 0.0);
        drawBush();
    }
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-48, 4.0, -18.5);
    drawBush();
    for (int i = 0; i <= 10; i++) {
        glTranslatef(10.0, 0.0, 0.0);
        drawBush();
    }
    glPopMatrix();
    // Draw cars
    // Car1
    glPushMatrix();
    glTranslatef(car1.x, 1.2, 0);
    glScalef(1.2, 1.2, 1.2);
    drawCar(0, 0, 0);
    glPopMatrix();

    // Car2
    glPushMatrix();
    glTranslatef(car2.x, 1.2, -38);
    glScalef(1.2, 1.2, 1.2);
    drawCar(0, 0, 0);
    glPopMatrix();

    glutSwapBuffers();
}

//This will be called when the viewport is changed
void myReshape(GLsizei w, GLsizei h)
{
    glViewport(0, 0, intWinWidth, intWinHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (intWinWidth <= intWinHeight)
        glOrtho(-1.5, 1.5, -1.5 * (GLfloat)intWinHeight / (GLfloat)intWinWidth, 1.5 * (GLfloat)intWinHeight / (GLfloat)intWinWidth, -10.0, 10.0);
    else
        glOrtho(-1.5 * (GLfloat)intWinWidth / (GLfloat)intWinHeight, 1.5 * (GLfloat)intWinWidth / (GLfloat)intWinHeight, -1.5, 1.5, -10.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Interactions
void mouseClick(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON) {
        lastMouseX = x;
        lastMouseY = y;
    }
    else if (button == GLUT_RIGHT_BUTTON) {
        lastMouseX = x;
        lastMouseY = y;
    }
}

void mouseWheel(int wheel, int direction, int x, int y)
{
    // Adjust the current distance according to the scrolling direction of the wheel
    if (direction > 0) {
        currentDistance -= 5.0f;
    }
    else {
        currentDistance += 5.0f;
    }

    // Keep a distance
    if (currentDistance < 10.0f) {
        currentDistance = 10.0f;
    }

    // The camera position is updated according to the new distance
    fltX0 = currentDistance * cos(angleY0 * M_PI / 180.0f);
    fltZ0 = currentDistance * sin(angleY0 * M_PI / 180.0f);

    glutPostRedisplay();
}

void mouseMotion(int x, int y)
{
    if (lastMouseX == -1 || lastMouseY == -1) {
        lastMouseX = x;
        lastMouseY = y;
        return;
    }
    int deltaX = x - lastMouseX;
    int deltaY = y - lastMouseY;

    // Limit the pitch Angle to be between -89 and 89 degrees
    angleY0 += deltaX * sensitivity; // Left and right rotation
    if (angleY0 > 89.0f) {
        angleY0 = 89.0f;
    }
    else if (angleY0 < -89.0f) {
        angleY0 = -89.0f;
    }

    // Limit the roll Angle between -180 and 180 degrees
    angleX0 -= deltaY * sensitivity; // Up and down rotation
    if (angleX0 > 180.0f) {
        angleX0 = 180.0f;
    }
    else if (angleX0 < -180.0f) {
        angleX0 = -180.0f;
    }
    lastMouseX = x;
    lastMouseY = y;

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y)
{
    switch (key) {
    case GLUT_KEY_UP:
        fltY0 += moveSpeed;
        break;
    case GLUT_KEY_DOWN:
        fltY0 -= moveSpeed;
        break;
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'q':
    case 'Q':
        exit(0); // Exit procedure
        break;
    case 'w':
    case 'W':
        fltZ0 -= moveSpeed * cos(angleY0 * M_PI / 180.0f); // Move the camera forward
        fltX0 -= moveSpeed * sin(angleY0 * M_PI / 180.0f);
        break;
    case 's':
    case 'S':
        fltZ0 += moveSpeed * cos(angleY0 * M_PI / 180.0f); // Move the camera backwards
        fltX0 += moveSpeed * sin(angleY0 * M_PI / 180.0f);
        break;
    case 'a':
    case 'A':
        fltZ0 -= moveSpeed * sin(angleY0 * M_PI / 180.0f); // Move the camera left
        fltX0 += moveSpeed * cos(angleY0 * M_PI / 180.0f);
        break;
    case 'd':
    case 'D':
        fltZ0 += moveSpeed * sin(angleY0 * M_PI / 180.0f); // Move the camera right
        fltX0 -= moveSpeed * cos(angleY0 * M_PI / 180.0f);
        break;
    case 'l':
    case 'L':
        currentState = static_cast<LightState>((currentState + 1) % 3);
        myLight();
        break;
    }
    glutPostRedisplay();
}

// Main function
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(intWinWidth, intWinHeight);
    glutCreateWindow("The Tesseract and the Yin-Yang Eight Diagrams");

    myLight();
    myTexture();

    glutDisplayFunc(display);
    glutReshapeFunc(myReshape);

    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseMotion);
    glutMouseWheelFunc(mouseWheel);

    glutIdleFunc(when_in_mainloop);
    glutTimerFunc(0, carTimer, 1);
    glutTimerFunc(16, flagTimer, 2);

    glutMainLoop();
    return 0;
}