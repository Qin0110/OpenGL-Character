#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>
#include <chrono>
#include <thread>
#include <iostream>

#define WINDOW_TITLE "Assignment Character"
#define MAX_DROPS 100

struct Raindrop {
	float x;
	float y;
	float speed;
	float length;
};

Raindrop rain[MAX_DROPS];

float oLeft = -20, oRight = 20, oBottom = -20, oTop = 20, oNear = -50, oFar = 100;
float ptx = 0, pty = 0, ptz = 0, pspeed = 0.1;
float p_rotate_angle = 0, p_angle_speed = 10;
float zoom = 1.0f, zoom_speed = 0.1f;
float left_leg_angle = 0, right_leg_angle = 0, maximum_leg_angle = 30, minimum_leg_angle = -30, leg_angle_speed = 5, leg_direction = 1;
int character_facing_angle = 0, character_rotation_speed = 90, target_facing_angle= 0;
float mx = 0, my = 0, mz = 0, mspeed = 1; //movement
bool armorOn = true;

// Attack Animation
bool isAttack = false;
float swingTargetShoulder = 60;
float swingTargetForearm = 80;
float swingSpeed = 5;
int swingForward = 0;
float attackPalmAngles = 0;
float attackWeaponSpear = 0;
float attackRing = 0;
bool ejectRing = false;

//Light
bool lightOn = TRUE;
bool textureOn = TRUE;

//perspective
float tz = 0.0;
float tSpeed = 0.5;
float prx = 0, pry = 0, prz = 0, prspeed = 10;
bool isOrtho = false;


//background texture mapping
BITMAP BMP;
HBITMAP hBMP = NULL;

GLuint roofTex;
GLuint redWallTex;
GLuint greyWallTex;
GLuint doorTex;
GLuint windowTex;
GLuint houseWallTex;
GLuint floorTex;
GLuint lanternTex;
GLuint pathWayTex;
GLuint tableTex;


double objRot = 0.0;

//hand animation
int selectedHand = 0;
int selectedFinger = 1;
int selectedJoint = 0;

//finger animation
float fingerAngles[2][5][3] = {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
	{
		{45, 30, 20}, // thumb
		{50, 45, 35}, // index finger
		{50, 45, 35}, // middle finger
		{50, 45, 35}, // ring finger
		{50, 45, 35}  // pinky finger
	}
};

//forearm animation
float forearmAngles[2] = { 0, 0 };

//shoulder animation
float shoulderAngles[2] = { 0, 0 };

// hand palm animation
float palmAngles[2] = { -90, 288 };
float palmMaxAngles[2] = { 0, 300 };
float palmMinAngles[2] = { -180, 198 };

//head animation 
float headAngle = 0;

LRESULT WINAPI WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		switch (wParam) {

		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case VK_SHIFT:
			selectedHand = 1;
			swingForward = 1;
			isAttack = !isAttack;
			forearmAngles[selectedHand] = 0;
			shoulderAngles[selectedHand] = 0;
			palmAngles[selectedHand] = 288; 
			attackPalmAngles = 0;
			attackWeaponSpear = 0;
			attackRing = 0;
			break;
		case VK_CONTROL:
			ejectRing = !ejectRing;
			attackRing = 0;
			break;
		case VK_UP:
			pty -= pspeed;
			break;
		case VK_DOWN:
			pty += pspeed;
			break;
		case VK_LEFT:
			ptx -= pspeed;
			break;
		case VK_RIGHT:
			ptx += pspeed;
			break;

		case '+':
		case VK_OEM_PLUS:
			if (isOrtho)
			{
				if (zoom > 0.5f)
				{
					zoom -= zoom_speed;
				}
			}
			else
			{
				if (tz < 0)
				{
					tz += tSpeed; // zoom in
				}
			}
			break;

		case '-':
		case VK_OEM_MINUS:
			if (isOrtho)
			{
				if (zoom < 3.0f)
				{
					zoom += zoom_speed;
				}
			}
			else
			{
				if (tz < 10)
				{
					tz -= tSpeed; // zoom out
				}
			}			
			break;

		case 'N':
			objRot -= 5.0; // spin left
			break;
		case 'M':
			objRot += 5.0; // spin right
			break;

		case 'O': 
			isOrtho = true; 
			tz = 0; 
			if (isOrtho)
			{
				zoom = 3.0f;
				prx = -45;
				pry = -45;
			}
			break; 
		case 'P': 
			isOrtho = false; 
			tz = 0; 
			zoom = 0.0;
			prx = 0;
			pry = 0;
			break;

		case 'S': {
			float rad = character_facing_angle * 3.14159f / 180.0f;
			float newMx = mx - sin(rad) * mspeed;
			float newMz = mz - cos(rad) * mspeed;

			bool inBounds;
			if (isOrtho) {
				inBounds = (newMz > -(oFar - 3) / 2);
			}
			else {
				inBounds = (newMx > -40 && newMx < 40 && 
					newMz > -30 && newMz < 100);     
			}

			if (inBounds) {
				mx = newMx;
				mz = newMz;
				left_leg_angle += (leg_angle_speed * leg_direction);
				if (left_leg_angle >= maximum_leg_angle) {
					left_leg_angle = maximum_leg_angle;
					leg_direction = -1;
				}
				else if (left_leg_angle <= minimum_leg_angle) {
					left_leg_angle = minimum_leg_angle;
					leg_direction = 1;
				}
				right_leg_angle = -left_leg_angle;
			}
			break;
		}
		case 'W': {
			float rad = character_facing_angle * 3.14159f / 180.0f;
			float newMx = mx + sin(rad) * mspeed; 
			float newMz = mz + cos(rad) * mspeed; 

			bool inBounds;
			if (isOrtho) {
				inBounds = (newMz < -(oNear - 5));
			}
			else {
				inBounds = (newMx > -40 && newMx < 40 &&  
					newMz > -30 && newMz < 100);    
			}

			if (inBounds) {
				mx = newMx;
				mz = newMz;
				left_leg_angle += (leg_angle_speed * leg_direction);
				if (left_leg_angle >= maximum_leg_angle) {
					left_leg_angle = maximum_leg_angle;
					leg_direction = -1;
				}
				else if (left_leg_angle <= minimum_leg_angle) {
					left_leg_angle = minimum_leg_angle;
					leg_direction = 1;
				}
				right_leg_angle = -left_leg_angle;
			}
			break;
		}

			// character face directon
		case 'A':
			target_facing_angle += character_rotation_speed;
			break;
		case 'D':
			target_facing_angle -= character_rotation_speed;
			break;

			//head animation
		case 'Q': // left
			if (headAngle > -80.0f) {
				headAngle -= 5.0f;
			}
			break;

		case 'E': // right
			if (headAngle < 80.0f) {
				headAngle += 5.0f;
			}
			break;

			//hand aanimation
				// hand
		case 'L': selectedHand = 0; break; // left hand
		case 'R': selectedHand = 1; break; // right hand

			//finger animation
			// finger 
		case 'Z': selectedFinger = 0; break; // thumb
		case 'X': selectedFinger = 1; break; // tndex
		case 'C': selectedFinger = 2; break; // middle
		case 'V': selectedFinger = 3; break; // ring
		case 'B': selectedFinger = 4; break; // pinky

			// finger joint 
		case '1': selectedJoint = 0; break; // base joint
		case '2': selectedJoint = 1; break; // middle Joint
		case '3': selectedJoint = 2; break; // tip joint
		case 'F':
			for (int f = 0; f < 5; f++) {
				for (int j = 0; j < 3; j++) {
					fingerAngles[selectedHand][f][j] = 50.0;
				}
			}
			break;

		case 'G':
			for (int f = 0; f < 5; f++) {
				for (int j = 0; j < 3; j++) {
					fingerAngles[selectedHand][f][j] = 0.0;
				}
			}
			break;
		case VK_PRIOR:
			if (fingerAngles[selectedHand][selectedFinger][selectedJoint] < 50.0) {
				fingerAngles[selectedHand][selectedFinger][selectedJoint] += 5.0;
			}
			break;

		case VK_NEXT:
			if (fingerAngles[selectedHand][selectedFinger][selectedJoint] > 0.0) {
				fingerAngles[selectedHand][selectedFinger][selectedJoint] -= 5.0;
			}
			break;

			//forearm animation
		case 'U':
			if (forearmAngles[selectedHand] < 115) {
				forearmAngles[selectedHand] += 5;
			}
			break;

		case 'J':
			if (forearmAngles[selectedHand] > -15) {
				forearmAngles[selectedHand] -= 5;
			}
			break;

			// shoulder animation
		case 'I':
			if (shoulderAngles[selectedHand] > -100) {
				shoulderAngles[selectedHand] -= 5;
			}
			break;

		case 'K':
			if (shoulderAngles[selectedHand] < 145) {
				shoulderAngles[selectedHand] += 5;
			}
			break;

			// hand palm animation
		case 'Y':
			if (palmAngles[selectedHand] < palmMaxAngles[selectedHand]) {
				palmAngles[selectedHand] += 5.0;
			}
			break;

		case 'H':
			if (palmAngles[selectedHand] > palmMinAngles[selectedHand]) {
				palmAngles[selectedHand] -= 5.0;
			}
			break;

		//Lighting switch
		case VK_F1:
			lightOn = !lightOn;
			break;

		case VK_F2:
			textureOn = !textureOn;
			break;

		//Camera viewing angle
		case '5':
			prx -= prspeed;
			break;
		case '6':
			prx += prspeed;
			break;
		case '7':
			pry += prspeed;
			break;
		case '8':
			pry -= prspeed;
			break;
		case '9':
			prz += prspeed;
			break;
		case '0':
			prz -= prspeed;
			break;

		case VK_F5:
			armorOn = !armorOn;
			break;

		// reset all
		case VK_SPACE:
			pty = 0, ptx = 0, ptz = 0;
			prx = 0, pry = 0, prz = 0;
			character_facing_angle = 0;
			target_facing_angle = 0;
			mx = 0, my = 0, mz = 0;
			left_leg_angle = 0, right_leg_angle = 0;
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
//--------------------------------------------------------------------

bool initPixelFormat(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.cAlphaBits = 8;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 0;

	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;

	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;

	// choose pixel format returns the number most similar pixel format available
	int n = ChoosePixelFormat(hdc, &pfd);

	// set pixel format returns whether it sucessfully set the pixel format
	if (SetPixelFormat(hdc, n, &pfd))
	{
		return true;
	}
	else
	{
		return false;
	}
}
//--------------------------------------------------------------------

void attacking() {
	//using namespace std::chrono_literals;
	if (!isAttack) return;

	if (swingForward == 1) {
		shoulderAngles[selectedHand] += swingSpeed;
		forearmAngles[selectedHand] += swingSpeed;
		palmAngles[selectedHand] -= 5;

		if (shoulderAngles[selectedHand] >= swingTargetShoulder ||
			forearmAngles[selectedHand] >= swingTargetForearm) {
			shoulderAngles[selectedHand] = swingTargetShoulder;
			forearmAngles[selectedHand] = swingTargetForearm;
			//std::this_thread::sleep_for(0.5s);
			swingForward = 2;
		}
	}
	else if (swingForward == 2) {
		shoulderAngles[selectedHand] -= swingSpeed;
		forearmAngles[selectedHand] -= swingSpeed;
		palmAngles[selectedHand] += 5;

		if (shoulderAngles[selectedHand] <= -30.0f ||
			forearmAngles[selectedHand] <= -20.0f) {
			shoulderAngles[selectedHand] = -30.0f;
			forearmAngles[selectedHand] = -20.0f;
			//std::this_thread::sleep_for(0.5s);
			swingForward = 3;
		}
	}
	else if (swingForward == 3)
	{
		bool done = true;
		if (shoulderAngles[selectedHand] < 30)
		{
			shoulderAngles[selectedHand] += swingSpeed + 5;
			done = false;
		}
		if (forearmAngles[selectedHand] < 50)
		{
			forearmAngles[selectedHand] += swingSpeed;
			done = false;
		}
		if (attackPalmAngles != -90)
		{
			attackPalmAngles -= (swingSpeed * 2);
			done = false;
		}
		if (attackWeaponSpear != 5)
		{
			attackWeaponSpear += 0.5;
		}
		if (done)
		{
			swingForward = 0;
		}
	}
	if (ejectRing && isAttack)
	{
		if (attackRing < 20)
		{
			attackRing += 1;
		}
	}
}

void initRain() {
	for (int i = 0; i < MAX_DROPS; i++) {
		// Random X position across the screen (-1.0 to 1.0)
		rain[i].x = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
	
		// Random Y starting position above the screen (1.0 to 2.0)
		// so they don't all appear at once
		rain[i].y = 1.0f + ((float)rand() / RAND_MAX);
	
		// Random fall speed
		rain[i].speed = 0.005f + ((float)rand() / RAND_MAX) * 0.01f;
	
		// Random drop length
		rain[i].length = 0.001f + ((float)rand() / RAND_MAX) * 0.005f;
	}
}

void drawRain() {
	// Save current matrices
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0); // NDC-style 2D space

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);  // rain shouldn't be lit
	glDisable(GL_DEPTH_TEST); // draw on top of everything

	glColor3f(1, 1, 1.0f); // light blue tint
	glLineWidth(4);

	for (int i = 0; i < MAX_DROPS; i++) {
		glBegin(GL_LINE_STRIP);
		glVertex2f(rain[i].x, rain[i].y);
		glVertex2f(rain[i].x, rain[i].y - rain[i].length);
		glEnd();

		rain[i].y -= rain[i].speed;

		if (rain[i].y < -1.0f) {
			rain[i].y = 1.0f;
			rain[i].x = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
		}
	}

	// Restore matrices and state
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
}

GLuint loadTexture(LPCSTR filepath) {
	GLUquadricObj* quadric = gluNewQuadric();
	gluQuadricTexture(quadric, GL_TRUE);
	gluQuadricNormals(quadric, GLU_SMOOTH);

	// Take from step 1
	GLuint texture = 0;

	// Initialize texture info
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	HBITMAP hBMP = (HBITMAP)LoadImage(GetModuleHandle(NULL),
		filepath, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION |
		LR_LOADFROMFILE);
	GetObject(hBMP, sizeof(BMP), &BMP);

	if (textureOn)
	{
		glEnable(GL_TEXTURE_2D);
	}
	else {
		glDisable(GL_TEXTURE_2D);
	}
	
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BMP.bmWidth,
		BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);

	// take from step 5
	DeleteObject(hBMP);

	return texture;
}



void disk(double intr, double outr) {
	GLUquadricObj* disk = NULL;
	disk = gluNewQuadric();

	gluQuadricTexture(disk, GL_TRUE);
	gluQuadricDrawStyle(disk, GLU_FILL);
	gluDisk(disk, intr, outr, 50, 50);

	gluDeleteQuadric(disk);
}

void cylinder(double base_Radius, double top_Radius, double height) {
	GLUquadricObj* cylinder = NULL;
	cylinder = gluNewQuadric();

	gluQuadricTexture(cylinder, GL_TRUE);
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	gluCylinder(cylinder, base_Radius, top_Radius, height, 50, 50);

	gluDeleteQuadric(cylinder);
}

void cone(double base_Radius, double height) {
	GLUquadricObj* cylinder = NULL;
	cylinder = gluNewQuadric();

	gluCylinder(cylinder, base_Radius, 0, height, 20, 20);

	gluDeleteQuadric(cylinder);
}

void sphere(double radius) {
	GLUquadricObj* sphere = NULL;
	sphere = gluNewQuadric();

	gluQuadricTexture(sphere, GL_TRUE);
	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluSphere(sphere, radius, 50, 50);

	gluDeleteQuadric(sphere);
}

void pyramid() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_TRIANGLES);

	// Front face
	glTexCoord2d(0, 1);
	glVertex3f(0.0f, 1.0f, 0.0f);   // Apex
	glTexCoord2d(0, 0);
	glVertex3f(-0.5f, 0.0f, 0.5f);   // Bottom-left
	glTexCoord2d(1, 1);
	glVertex3f(0.5f, 0.0f, 0.5f);   // Bottom-right

	// Back face
	glTexCoord2d(0, 1);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glTexCoord2d(0, 0);
	glVertex3f(0.5f, 0.0f, -0.5f);
	glTexCoord2d(1, 1);
	glVertex3f(-0.5f, 0.0f, -0.5f);

	// Left face
	glTexCoord2d(0, 1);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glTexCoord2d(0, 0);
	glVertex3f(-0.5f, 0.0f, -0.5f);
	glTexCoord2d(1, 1);
	glVertex3f(-0.5f, 0.0f, 0.5f);

	// Right face
	glTexCoord2d(0, 1);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glTexCoord2d(0, 0);
	glVertex3f(0.5f, 0.0f, 0.5f);
	glTexCoord2d(1, 1);
	glVertex3f(0.5f, 0.0f, -0.5f);

	glEnd();

	// Square base using GL_QUADS
	glBegin(GL_QUADS);
	glTexCoord2d(0, 1);
	glVertex3f(-0.5f, 0.0f, 0.5f);
	glTexCoord2d(1, 1);
	glVertex3f(0.5f, 0.0f, 0.5f);
	glTexCoord2d(1, 0);
	glVertex3f(0.5f, 0.0f, -0.5f);
	glTexCoord2d(0, 0);
	glVertex3f(-0.5f, 0.0f, -0.5f);
	glEnd();
}


void drawBox(double w, double h, double d) {
	glBegin(GL_QUADS);
	// front
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(-w, -h, d);
	glVertex3f(w, -h, d);
	glVertex3f(w, h, d);
	glVertex3f(-w, h, d);

	// back
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(-w, -h, -d);
	glVertex3f(-w, h, -d);
	glVertex3f(w, h, -d);
	glVertex3f(w, -h, -d);

	// top
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(-w, h, -d);
	glVertex3f(-w, h, d);
	glVertex3f(w, h, d);
	glVertex3f(w, h, -d);

	// bottom
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(-w, -h, -d);
	glVertex3f(w, -h, -d);
	glVertex3f(w, -h, d);
	glVertex3f(-w, -h, d);

	// right
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(w, -h, -d);
	glVertex3f(w, h, -d);
	glVertex3f(w, h, d);
	glVertex3f(w, -h, d);

	// left
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(-w, -h, -d);
	glVertex3f(-w, -h, d);
	glVertex3f(-w, h, d);
	glVertex3f(-w, h, -d);
	glEnd();
}

void door(float zPos) {
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, zPos);

	// door
	glColor3f(0.2f, 0.1f, 0.05f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-1.55f, -2.0f, 0.0f);
	glVertex3f(1.55f, -2.0f, 0.0f);
	glVertex3f(1.55f, 1.75f, 0.0f);
	glVertex3f(-1.55f, 1.75f, 0.0f);
	glEnd();

	// door panels
	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, doorTex);
		glColor3f(1.0f, 1.0f, 1.0f);
	}
	else {
		glDisable(GL_TEXTURE_2D);
	}

	glBegin(GL_QUADS);
	// left
	glNormal3f(0.0f, 0.0f, 1.0f);
	if (!textureOn) glColor3f(0.5f, 0.2f, 0.1f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.5f, -2.0f, 0.01f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.02f, -2.0f, 0.01f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.02f, 1.7f, 0.01f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.5f, 1.7f, 0.01f);

	// right
	if (!textureOn) glColor3f(0.6f, 0.3f, 0.15f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.02f, -2.0f, 0.01f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.5f, -2.0f, 0.01f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.5f, 1.7f, 0.01f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.02f, 1.7f, 0.01f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}

void houseWindow(float xPos, float yPos, float zPos) {
	glPushMatrix();

	glTranslatef(xPos, yPos, zPos);

	glScalef(1.5f, 1.5f, 1.0f);

	// window
	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, windowTex);
		glColor3f(1.0f, 1.0f, 1.0f);
	}
	else {
		glDisable(GL_TEXTURE_2D);
	}

	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	if (!textureOn) glColor3f(0.6f, 0.8f, 0.9f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.6f, -0.7f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.6f, -0.7f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.6f, 0.7f, 0.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.6f, 0.7f, 0.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	// window frame
	glColor3f(0.2f, 0.1f, 0.05f);
	glLineWidth(3.0f);
	glBegin(GL_LINES);

	// outer line
	glVertex3f(-0.6f, -0.7f, 0.01f); glVertex3f(0.6f, -0.7f, 0.01f); // bottom line
	glVertex3f(-0.6f, 0.7f, 0.01f); glVertex3f(0.6f, 0.7f, 0.01f); // top line
	glVertex3f(-0.6f, -0.7f, 0.01f); glVertex3f(-0.6f, 0.7f, 0.01f); // left line
	glVertex3f(0.6f, -0.7f, 0.01f); glVertex3f(0.6f, 0.7f, 0.01f); // right line

	// middle line
	glVertex3f(0.0f, -0.7f, 0.01f); glVertex3f(0.0f, 0.7f, 0.01f);
	glEnd();
	glLineWidth(1.0f);

	glPopMatrix();
}

void lantern(float x, float y, float z) {
	GLUquadric* sphereObj = gluNewQuadric();
	glPushMatrix();
	glTranslatef(x, y, z);

	// hanging string
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.2, 0.0, 0.0);
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, -0.5f, 0.0f);
	glEnd();
	glLineWidth(1.0f);

	// lattern
	glPushMatrix();
	glTranslatef(0.0f, -0.8f, 0.0f);
	glRotatef(90.0f, 1, 0, 0);
	glScalef(1.0f, 0.8f, 1.0f);

	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, lanternTex);
		glColor3f(1,0,0);
	}
	else {
		glDisable(GL_TEXTURE_2D);
		glColor3f(0.9f, 0.1f, 0.1f);
	}

	gluQuadricTexture(sphereObj, GL_TRUE);
	gluQuadricNormals(sphereObj, GLU_SMOOTH);
	gluSphere(sphereObj, 0.4f, 20, 20);

	gluDeleteQuadric(sphereObj);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	// bottom tassel 
	glColor3f(0.8f, 0.6f, 0.1f);

	glBegin(GL_QUADS);
	glVertex3f(-0.05f, -1.12f, 0.0f);
	glVertex3f(0.05f, -1.12f, 0.0f);
	glVertex3f(0.05f, -1.2f, 0.0f);
	glVertex3f(-0.05f, -1.2f, 0.0f);
	glEnd();

	// hanging fringe of the tassel
	glBegin(GL_TRIANGLES);
	glVertex3f(0.0f, -1.2f, 0.0f);
	glVertex3f(-0.1f, -1.5f, 0.0f);
	glVertex3f(0.1f, -1.5f, 0.0f);
	glEnd();

	glPopMatrix();
}

void gardenTable(float x, float y, float z) {
	GLUquadric* q = gluNewQuadric();
	glPushMatrix();
	glTranslatef(x, y, z);

	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tableTex);
		glColor3f(1.0f, 1.0f, 1.0f);
	}
	else {
		glDisable(GL_TEXTURE_2D);
	}
	gluQuadricTexture(q, GL_TRUE);

	// table bottom
	if (!textureOn) glColor3f(0.5f, 0.5f, 0.5f);
	gluCylinder(q, 0.3f, 0.3f, 0.8f, 20, 1);

	// top + side
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.8f);
	if (!textureOn) glColor3f(0.6f, 0.6f, 0.6f);
	gluDisk(q, 0.0f, 0.8f, 20, 1);
	gluCylinder(q, 0.8f, 0.8f, 0.1f, 20, 1);
	glTranslatef(0.0f, 0.0f, 0.1f);
	if (!textureOn) glColor3f(0.65f, 0.65f, 0.65f);
	gluDisk(q, 0.0f, 0.8f, 20, 1);
	glPopMatrix();

	// stools
	float stoolRadius = 1.2f;
	for (int i = 0; i < 4; i++) {
		glPushMatrix();
		float angle = i * (3.14159f / 2.0f);
		glTranslatef(cos(angle) * stoolRadius, sin(angle) * stoolRadius, 0.0f);
		if (!textureOn) glColor3f(0.4f, 0.4f, 0.4f);
		gluCylinder(q, 0.25f, 0.25f, 0.45f, 15, 1);
		glTranslatef(0.0f, 0.0f, 0.45f);
		if (!textureOn) glColor3f(0.45f, 0.45f, 0.45f);
		gluDisk(q, 0.0f, 0.25f, 15, 1);
		glPopMatrix();
	}

	gluDeleteQuadric(q);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void house() {
	glPushMatrix();

	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, houseWallTex);
		glColor3f(1.0f, 1.0f, 1.0f);
	}
	else {
		glDisable(GL_TEXTURE_2D);
	}

	//wall
	glBegin(GL_QUADS);

	//front
	glNormal3f(0.0f, 0.0f, 1.0f);
	if (!textureOn) glColor3f(0.8f, 0.8f, 0.8f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.5f, -2.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(6.5f, -2.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(6.5f, 3.5f, 0.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-6.5f, 3.5f, 0.0f);

	//back
	glNormal3f(0.0f, 0.0f, -1.0f);
	if (!textureOn) glColor3f(0.7f, 0.7f, 0.7f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(6.5f, -2.0f, -6.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-6.5f, -2.0f, -6.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-6.5f, 3.5f, -6.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(6.5f, 3.5f, -6.0f);

	// right
	glNormal3f(1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.75f, 0.75f, 0.75f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(6.5f, -2.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(6.5f, -2.0f, -6.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(6.5f, 3.5f, -6.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(6.5f, 3.5f, 0.0f);

	//left
	glNormal3f(-1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.75f, 0.75f, 0.75f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.5f, -2.0f, -6.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-6.5f, -2.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-6.5f, 3.5f, 0.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-6.5f, 3.5f, -6.0f);

	//top
	glNormal3f(0.0f, 1.0f, 0.0f);
	if (!textureOn) glColor3f(0.9f, 0.9f, 0.9f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.5f, 3.5f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(6.5f, 3.5f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(6.5f, 3.5f, -6.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-6.5f, 3.5f, -6.0f);

	//bottom
	glNormal3f(0.0f, -1.0f, 0.0f);
	if (!textureOn) glColor3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-6.5f, -2.0f, -6.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(6.5f, -2.0f, -6.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(6.5f, -2.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-6.5f, -2.0f, 0.0f);
	glEnd();

	// roof eaves
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.15f, 0.15f, 0.15f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-7.3f, 3.5f, 0.8f);
	glVertex3f(7.3f, 3.5f, 0.8f);
	glVertex3f(7.3f, 3.5f, -6.8f);
	glVertex3f(-7.3f, 3.5f, -6.8f);
	glEnd();

	//roof slants
	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, roofTex);
		glColor3f(1.0f, 1.0f, 1.0f);
	}
	else {
		glDisable(GL_TEXTURE_2D);
	}

	glBegin(GL_QUADS);
	//front 
	glNormal3f(0.0f, 0.707f, 0.707f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-7.3f, 3.5f, 0.8f);
	glTexCoord2f(12.0f, 0.0f); glVertex3f(7.3f, 3.5f, 0.8f);
	glTexCoord2f(12.0f, 4.0f); glVertex3f(7.3f, 4.9f, -3.0f);
	glTexCoord2f(0.0f, 4.0f); glVertex3f(-7.3f, 4.9f, -3.0f);

	// back
	glNormal3f(0.0f, 0.707f, -0.707f);
	if (!textureOn) glColor3f(0.15f, 0.15f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(7.3f, 3.5f, -6.8f);
	glTexCoord2f(12.0f, 0.0f); glVertex3f(-7.3f, 3.5f, -6.8f);
	glTexCoord2f(12.0f, 4.0f); glVertex3f(-7.3f, 4.9f, -3.0f);
	glTexCoord2f(0.0f, 4.0f); glVertex3f(7.3f, 4.9f, -3.0f);
	glEnd();

	glBegin(GL_TRIANGLES);
	//right
	glNormal3f(1.0f, 0.5f, 0.0f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(7.3f, 3.5f, 0.8f);
	glTexCoord2f(7.6f, 0.0f); glVertex3f(7.3f, 3.5f, -6.8f);
	glTexCoord2f(3.8f, 4.0f); glVertex3f(7.3f, 4.9f, -3.0f);

	//left
	glNormal3f(-1.0f, 0.5f, 0.0f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-7.3f, 3.5f, -6.8f);
	glTexCoord2f(7.6f, 0.0f); glVertex3f(-7.3f, 3.5f, 0.8f);
	glTexCoord2f(3.8f, 4.0f); glVertex3f(-7.3f, 4.9f, -3.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	//door
	door(0.01f);

	//window
	houseWindow(-4.0f, -0.2f, 0.01f); //left
	houseWindow(4.0f, -0.2f, 0.01f); //right

	lantern(-4.0f, 3.5f, 0.6f); // left lantern
	lantern(4.0f, 3.5f, 0.6f);  // right

	glPopMatrix();
}

void background() {
	const float PI = 3.14159265f;
	int numSegments = 20;
	float radiusX = 1.5f;
	float radiusY = 1.0f;
	float baseY = 1.5f;
	glPushMatrix();

	//roof eaves
	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, floorTex);
		glColor3f(1.0f, 1.0f, 1.0f);
	}
	else {
		glDisable(GL_TEXTURE_2D);
	}

	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);
	if (!textureOn) glColor3f(0.4f, 0.4f, 0.4f);
	glTexCoord2f(0.0f, 0.0f);   glVertex3f(-24.0f, -0.8f, 24.0f);
	glTexCoord2f(24.0f, 0.0f);  glVertex3f(24.0f, -0.8f, 24.0f);
	glTexCoord2f(24.0f, 24.0f); glVertex3f(24.0f, -0.8f, -24.0f);
	glTexCoord2f(0.0f, 24.0f);  glVertex3f(-24.0f, -0.8f, -24.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	//wall
	glPushMatrix();

	glScalef(1.5f, 1.2f, 1.5f);

	//1. middle wall (left)
	glDisable(GL_TEXTURE_2D);
	//slants bottom
	glBegin(GL_QUADS);
	glColor3f(0.15f, 0.15f, 0.15f);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-7.0f, 3.0f, -7.0f);
	glVertex3f(-4.3f, 3.0f, -7.0f);
	glVertex3f(-4.3f, 3.0f, -10.0f);
	glVertex3f(-10.0f, 3.0f, -10.0f);
	glEnd();

	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, greyWallTex);
	}
	else {
		glDisable(GL_TEXTURE_2D);
	}

	//grey wall
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	if (!textureOn) glColor3f(0.6f, 0.6f, 0.6f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(-4.3f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(-4.3f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -7.5f);

	glNormal3f(0, 0, -1);
	if (!textureOn) glColor3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.8f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(-4.3f, -0.8f, -9.5f);

	glNormal3f(0, 1, 0);
	if (!textureOn) glColor3f(0.65f, 0.65f, 0.65f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -7.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(-4.3f, -0.2f, -7.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -9.5f);

	glNormal3f(0, -1, 0);
	if (!textureOn) glColor3f(0.45f, 0.45f, 0.45f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(-4.3f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(-4.3f, -0.8f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.8f, -9.5f);

	glNormal3f(1, 0, 0);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.3f, -0.8f, -7.5f);
	glTexCoord2f(2.0f, 0.0f); glVertex3f(-4.3f, -0.8f, -9.5f);
	glTexCoord2f(2.0f, 1.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.3f, -0.2f, -7.5f);

	glNormal3f(-1, 0, 0);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.8f, -9.5f);
	glTexCoord2f(2.0f, 0.0f); glVertex3f(-9.5f, -0.8f, -7.5f);
	glTexCoord2f(2.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glEnd();

	//red wall
	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, redWallTex);
	}
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	if (!textureOn) glColor3f(0.7f, 0.2f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-4.3f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.3f, 3.0f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, 3.0f, -7.5f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	if (!textureOn) glColor3f(0.6f, 0.15f, 0.15f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-9.5f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.3f, 3.0f, -9.5f);

	glNormal3f(0.0f, 1.0f, 0.0f);
	if (!textureOn) glColor3f(0.75f, 0.25f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, 3.0f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-4.3f, 3.0f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.3f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, 3.0f, -9.5f);

	glNormal3f(0.0f, -1.0f, 0.0f);
	if (!textureOn) glColor3f(0.5f, 0.1f, 0.1f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.3f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -7.5f);

	glNormal3f(1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.3f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.3f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.3f, 3.0f, -7.5f);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-9.5f, 3.0f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, 3.0f, -9.5f);
	glEnd();

	//roof 
	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, roofTex);
	}
	glBegin(GL_QUADS);
	//inside
	glNormal3f(0.0f, 0.707f, 0.707f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-7.0f, 3.0f, -7.0f);
	glTexCoord2f(3.0f, 0.0f); glVertex3f(-4.3f, 3.0f, -7.0f);
	glTexCoord2f(3.0f, 1.0f); glVertex3f(-4.3f, 3.8f, -8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-8.5f, 3.8f, -8.5f);

	//outside
	glNormal3f(0.0f, 0.707f, -0.707f);
	if (!textureOn) glColor3f(0.15f, 0.15f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 3.0f, -10.0f);
	glTexCoord2f(6.0f, 0.0f); glVertex3f(-4.3f, 3.0f, -10.0f);
	glTexCoord2f(6.0f, 1.0f); glVertex3f(-4.3f, 3.8f, -8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-8.5f, 3.8f, -8.5f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//2. middle wall (right)
	glBegin(GL_QUADS);
	//roof eaves
	glColor3f(0.15f, 0.15f, 0.15f);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(4.3f, 3.0f, -7.0f);
	glVertex3f(7.0f, 3.0f, -7.0f);
	glVertex3f(10.0f, 3.0f, -10.0f);
	glVertex3f(4.3f, 3.0f, -10.0f);
	glEnd();

	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, greyWallTex);
	}

	//grey wall
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	if (!textureOn) glColor3f(0.6f, 0.6f, 0.6f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(9.5f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(9.5f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, -0.2f, -7.5f);

	glNormal3f(0, 0, -1);
	if (!textureOn) glColor3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.8f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, -0.2f, -9.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(9.5f, -0.8f, -9.5f);

	glNormal3f(0, 1, 0);
	if (!textureOn) glColor3f(0.65f, 0.65f, 0.65f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.2f, -7.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(9.5f, -0.2f, -7.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, -0.2f, -9.5f);

	glNormal3f(0, -1, 0);
	if (!textureOn) glColor3f(0.45f, 0.45f, 0.45f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(9.5f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(9.5f, -0.8f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, -0.8f, -9.5f);

	glNormal3f(1, 0, 0);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(9.5f, -0.8f, -7.5f);
	glTexCoord2f(2.0f, 0.0f); glVertex3f(9.5f, -0.8f, -9.5f);
	glTexCoord2f(2.0f, 1.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(9.5f, -0.2f, -7.5f);

	glNormal3f(-1, 0, 0);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.8f, -9.5f);
	glTexCoord2f(2.0f, 0.0f); glVertex3f(4.3f, -0.8f, -7.5f);
	glTexCoord2f(2.0f, 1.0f); glVertex3f(4.3f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, -0.2f, -9.5f);
	glEnd();

	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, redWallTex);
	}
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	if (!textureOn) glColor3f(0.7f, 0.2f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(9.5f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(9.5f, 3.0f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, 3.0f, -7.5f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	if (!textureOn) glColor3f(0.6f, 0.15f, 0.15f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(4.3f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(9.5f, 3.0f, -9.5f);

	glNormal3f(0.0f, 1.0f, 0.0f);
	if (!textureOn) glColor3f(0.75f, 0.25f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, 3.0f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(9.5f, 3.0f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(9.5f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, 3.0f, -9.5f);

	glNormal3f(0.0f, -1.0f, 0.0f);
	if (!textureOn) glColor3f(0.5f, 0.1f, 0.1f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(9.5f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, -0.2f, -7.5f);

	glNormal3f(1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(9.5f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(9.5f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(9.5f, 3.0f, -7.5f);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(4.3f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(4.3f, 3.0f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, 3.0f, -9.5f);
	glEnd();

	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, roofTex);
	}
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.707f, 0.707f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, 3.0f, -7.0f);
	glTexCoord2f(3.0f, 0.0f); glVertex3f(7.0f, 3.0f, -7.0f);
	glTexCoord2f(3.0f, 1.0f); glVertex3f(8.5f, 3.8f, -8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, 3.8f, -8.5f);

	glNormal3f(0.0f, 0.707f, -0.707f);
	if (!textureOn) glColor3f(0.15f, 0.15f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, 3.0f, -10.0f);
	glTexCoord2f(6.0f, 0.0f); glVertex3f(10.0f, 3.0f, -10.0f);
	glTexCoord2f(6.0f, 1.0f); glVertex3f(8.5f, 3.8f, -8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, 3.8f, -8.5f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//3. right wall
	glBegin(GL_QUADS);
	glColor3f(0.15f, 0.15f, 0.15f);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(7.0f, 3.0f, 10.0f);
	glVertex3f(7.0f, 3.0f, -10.0f);
	glVertex3f(10.0f, 3.0f, -10.0f);
	glVertex3f(10.0f, 3.0f, 10.0f);
	glEnd();

	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, greyWallTex);
	}

	glBegin(GL_QUADS);
	glNormal3f(-1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.6f, 0.6f, 0.6f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(7.5f, -0.8f, -9.5f);
	glTexCoord2f(8.0f, 0.0f); glVertex3f(7.5f, -0.8f, 9.5f);
	glTexCoord2f(8.0f, 1.0f); glVertex3f(7.5f, -0.2f, 9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(7.5f, -0.2f, -9.5f);

	glNormal3f(1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(9.5f, -0.8f, 9.5f);
	glTexCoord2f(8.0f, 0.0f); glVertex3f(9.5f, -0.8f, -9.5f);
	glTexCoord2f(8.0f, 1.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(9.5f, -0.2f, 9.5f);

	glNormal3f(0.0f, 1.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.65f, 0.65f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(7.5f, -0.2f, 9.5f);
	glTexCoord2f(8.0f, 0.0f); glVertex3f(9.5f, -0.2f, 9.5f);
	glTexCoord2f(8.0f, 1.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(7.5f, -0.2f, -9.5f);

	glNormal3f(0.0f, -1.0f, 0.0f);
	if (!textureOn) glColor3f(0.45f, 0.45f, 0.45f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(7.5f, -0.8f, -9.5f);
	glTexCoord2f(8.0f, 0.0f); glVertex3f(9.5f, -0.8f, -9.5f);
	glTexCoord2f(8.0f, 1.0f); glVertex3f(9.5f, -0.8f, 9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(7.5f, -0.8f, 9.5f);

	glNormal3f(0.0f, 0.0f, 1.0f);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(7.5f, -0.8f, 9.5f);
	glTexCoord2f(8.0f, 0.0f); glVertex3f(9.5f, -0.8f, 9.5f);
	glTexCoord2f(8.0f, 1.0f); glVertex3f(9.5f, -0.2f, 9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(7.5f, -0.2f, 9.5f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(9.5f, -0.8f, -9.5f);
	glTexCoord2f(8.0f, 0.0f); glVertex3f(7.5f, -0.8f, -9.5f);
	glTexCoord2f(8.0f, 1.0f); glVertex3f(7.5f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glEnd();

	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, redWallTex);
	}
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	if (!textureOn) glColor3f(0.7f, 0.2f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(7.5f, -0.2f, 9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(9.5f, -0.2f, 9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(9.5f, 3.0f, 9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(7.5f, 3.0f, 9.5f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	if (!textureOn) glColor3f(0.6f, 0.15f, 0.15f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(7.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(7.5f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(9.5f, 3.0f, -9.5f);

	glNormal3f(0.0f, 1.0f, 0.0f);
	if (!textureOn) glColor3f(0.75f, 0.25f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(7.5f, 3.0f, 9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(9.5f, 3.0f, 9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(9.5f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(7.5f, 3.0f, -9.5f);

	glNormal3f(0.0f, -1.0f, 0.0f);
	if (!textureOn) glColor3f(0.5f, 0.1f, 0.1f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(7.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(9.5f, -0.2f, 9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(7.5f, -0.2f, 9.5f);

	glNormal3f(1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(9.5f, -0.2f, 9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(9.5f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(9.5f, 3.0f, 9.5f);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(7.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(7.5f, -0.2f, 9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(7.5f, 3.0f, 9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(7.5f, 3.0f, -9.5f);
	glEnd();

	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, roofTex);
	}
	glBegin(GL_QUADS);
	glNormal3f(-0.707f, 0.707f, 0.0f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(7.0f, 3.0f, 10.0f);
	glTexCoord2f(4.0f, 0.0f); glVertex3f(7.0f, 3.0f, -10.0f);
	glTexCoord2f(4.0f, 1.0f); glVertex3f(8.5f, 3.8f, -8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(8.5f, 3.8f, 8.5f);

	glNormal3f(0.707f, 0.707f, 0.0f);
	if (!textureOn) glColor3f(0.15f, 0.15f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(10.0f, 3.0f, -10.0f);
	glTexCoord2f(4.0f, 0.0f); glVertex3f(10.0f, 3.0f, 10.0f);
	glTexCoord2f(4.0f, 1.0f); glVertex3f(8.5f, 3.8f, 8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(8.5f, 3.8f, -8.5f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3f(0.0f, 0.882f, 0.470f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(7.0f, 3.0f, 10.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(10.0f, 3.0f, 10.0f);
	glTexCoord2f(0.5f, 1.0f); glVertex3f(8.5f, 3.8f, 8.5f);

	glNormal3f(0.0f, 0.882f, -0.470f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(10.0f, 3.0f, -10.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(7.0f, 3.0f, -10.0f);
	glTexCoord2f(0.5f, 1.0f); glVertex3f(8.5f, 3.8f, -8.5f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//4. left wall
	glBegin(GL_QUADS);
	glColor3f(0.15f, 0.15f, 0.15f);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-10.0f, 3.0f, 10.0f);
	glVertex3f(-10.0f, 3.0f, -10.0f);
	glVertex3f(-7.0f, 3.0f, -10.0f);
	glVertex3f(-7.0f, 3.0f, 10.0f);
	glEnd();

	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, greyWallTex);
	}

	glBegin(GL_QUADS);
	glNormal3f(1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.6f, 0.6f, 0.6f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-7.5f, -0.8f, 9.5f);
	glTexCoord2f(8.0f, 0.0f); glVertex3f(-7.5f, -0.8f, -9.5f);
	glTexCoord2f(8.0f, 1.0f); glVertex3f(-7.5f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-7.5f, -0.2f, 9.5f);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.8f, -9.5f);
	glTexCoord2f(8.0f, 0.0f); glVertex3f(-9.5f, -0.8f, 9.5f);
	glTexCoord2f(8.0f, 1.0f); glVertex3f(-9.5f, -0.2f, 9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -9.5f);

	glNormal3f(0.0f, 1.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.65f, 0.65f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.2f, 9.5f);
	glTexCoord2f(8.0f, 0.0f); glVertex3f(-7.5f, -0.2f, 9.5f);
	glTexCoord2f(8.0f, 1.0f); glVertex3f(-7.5f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -9.5f);

	glNormal3f(0.0f, -1.0f, 0.0f);
	if (!textureOn) glColor3f(0.45f, 0.45f, 0.45f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.8f, -9.5f);
	glTexCoord2f(8.0f, 0.0f); glVertex3f(-7.5f, -0.8f, -9.5f);
	glTexCoord2f(8.0f, 1.0f); glVertex3f(-7.5f, -0.8f, 9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.8f, 9.5f);

	glNormal3f(0.0f, 0.0f, 1.0f);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.8f, 9.5f);
	glTexCoord2f(8.0f, 0.0f); glVertex3f(-7.5f, -0.8f, 9.5f);
	glTexCoord2f(8.0f, 1.0f); glVertex3f(-7.5f, -0.2f, 9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, 9.5f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-7.5f, -0.8f, -9.5f);
	glTexCoord2f(8.0f, 0.0f); glVertex3f(-9.5f, -0.8f, -9.5f);
	glTexCoord2f(8.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-7.5f, -0.2f, -9.5f);
	glEnd();

	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, redWallTex);
	}
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	if (!textureOn) glColor3f(0.7f, 0.2f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.2f, 9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-7.5f, -0.2f, 9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-7.5f, 3.0f, 9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, 3.0f, 9.5f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	if (!textureOn) glColor3f(0.6f, 0.15f, 0.15f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-7.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-9.5f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-7.5f, 3.0f, -9.5f);

	glNormal3f(0.0f, 1.0f, 0.0f);
	if (!textureOn) glColor3f(0.75f, 0.25f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, 3.0f, 9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-7.5f, 3.0f, 9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-7.5f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, 3.0f, -9.5f);

	glNormal3f(0.0f, -1.0f, 0.0f);
	if (!textureOn) glColor3f(0.5f, 0.1f, 0.1f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-7.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-7.5f, -0.2f, 9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, 9.5f);

	glNormal3f(1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-7.5f, -0.2f, 9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-7.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-7.5f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-7.5f, 3.0f, 9.5f);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-9.5f, -0.2f, 9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-9.5f, 3.0f, 9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, 3.0f, -9.5f);
	glEnd();

	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, roofTex);
	}
	glBegin(GL_QUADS);
	glNormal3f(0.707f, 0.707f, 0.0f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-7.0f, 3.0f, -10.0f);
	glTexCoord2f(4.0f, 0.0f); glVertex3f(-7.0f, 3.0f, 10.0f);
	glTexCoord2f(4.0f, 1.0f); glVertex3f(-8.5f, 3.8f, 8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-8.5f, 3.8f, -8.5f);

	glNormal3f(-0.707f, 0.707f, 0.0f);
	if (!textureOn) glColor3f(0.15f, 0.15f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 3.0f, 10.0f);
	glTexCoord2f(4.0f, 0.0f); glVertex3f(-10.0f, 3.0f, -10.0f);
	glTexCoord2f(4.0f, 1.0f); glVertex3f(-8.5f, 3.8f, -8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-8.5f, 3.8f, 8.5f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3f(0.0f, 0.882f, 0.470f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 3.0f, 10.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-7.0f, 3.0f, 10.0f);
	glTexCoord2f(0.5f, 1.0f); glVertex3f(-8.5f, 3.8f, 8.5f);
	glNormal3f(0.0f, 0.882f, -0.470f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-7.0f, 3.0f, -10.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-10.0f, 3.0f, -10.0f);
	glTexCoord2f(0.5f, 1.0f); glVertex3f(-8.5f, 3.8f, -8.5f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//5. front wall (left)
	glPushMatrix();
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor3f(0.15f, 0.15f, 0.15f);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-7.0f, 3.0f, -7.0f);
	glVertex3f(-4.3f, 3.0f, -7.0f);
	glVertex3f(-4.3f, 3.0f, -10.0f);
	glVertex3f(-10.0f, 3.0f, -10.0f);
	glEnd();

	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, greyWallTex);
	}

	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	if (!textureOn) glColor3f(0.6f, 0.6f, 0.6f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(-4.3f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(-4.3f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -7.5f);

	glNormal3f(0, 0, -1);
	if (!textureOn) glColor3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.8f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(-4.3f, -0.8f, -9.5f);

	glNormal3f(0, 1, 0);
	if (!textureOn) glColor3f(0.65f, 0.65f, 0.65f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -7.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(-4.3f, -0.2f, -7.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -9.5f);

	glNormal3f(0, -1, 0);
	if (!textureOn) glColor3f(0.45f, 0.45f, 0.45f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(-4.3f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(-4.3f, -0.8f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.8f, -9.5f);

	glNormal3f(1, 0, 0);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.3f, -0.8f, -7.5f);
	glTexCoord2f(2.0f, 0.0f); glVertex3f(-4.3f, -0.8f, -9.5f);
	glTexCoord2f(2.0f, 1.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.3f, -0.2f, -7.5f);

	glNormal3f(-1, 0, 0);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.8f, -9.5f);
	glTexCoord2f(2.0f, 0.0f); glVertex3f(-9.5f, -0.8f, -7.5f);
	glTexCoord2f(2.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glEnd();

	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, redWallTex);
	}
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	if (!textureOn) glColor3f(0.7f, 0.2f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-4.3f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.3f, 3.0f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, 3.0f, -7.5f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	if (!textureOn) glColor3f(0.6f, 0.15f, 0.15f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-9.5f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.3f, 3.0f, -9.5f);

	glNormal3f(0.0f, 1.0f, 0.0f);
	if (!textureOn) glColor3f(0.75f, 0.25f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, 3.0f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-4.3f, 3.0f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.3f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, 3.0f, -9.5f);

	glNormal3f(0.0f, -1.0f, 0.0f);
	if (!textureOn) glColor3f(0.5f, 0.1f, 0.1f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.3f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, -0.2f, -7.5f);

	glNormal3f(1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.3f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.3f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.3f, 3.0f, -7.5f);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-9.5f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-9.5f, 3.0f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-9.5f, 3.0f, -9.5f);
	glEnd();

	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, roofTex);
	}
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.707f, 0.707f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-7.0f, 3.0f, -7.0f);
	glTexCoord2f(3.0f, 0.0f); glVertex3f(-4.3f, 3.0f, -7.0f);
	glTexCoord2f(3.0f, 1.0f); glVertex3f(-4.3f, 3.8f, -8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-8.5f, 3.8f, -8.5f);

	glNormal3f(0.0f, 0.707f, -0.707f);
	if (!textureOn) glColor3f(0.15f, 0.15f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 3.0f, -10.0f);
	glTexCoord2f(6.0f, 0.0f); glVertex3f(-4.3f, 3.0f, -10.0f);
	glTexCoord2f(6.0f, 1.0f); glVertex3f(-4.3f, 3.8f, -8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-8.5f, 3.8f, -8.5f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	// 6. front wall (right)
	glBegin(GL_QUADS);
	glColor3f(0.15f, 0.15f, 0.15f);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(4.3f, 3.0f, -7.0f);
	glVertex3f(7.0f, 3.0f, -7.0f);
	glVertex3f(10.0f, 3.0f, -10.0f);
	glVertex3f(4.3f, 3.0f, -10.0f);
	glEnd();

	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, greyWallTex);
	}

	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	if (!textureOn) glColor3f(0.6f, 0.6f, 0.6f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(9.5f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(9.5f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, -0.2f, -7.5f);

	glNormal3f(0, 0, -1);
	if (!textureOn) glColor3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.8f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, -0.2f, -9.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(9.5f, -0.8f, -9.5f);

	glNormal3f(0, 1, 0);
	if (!textureOn) glColor3f(0.65f, 0.65f, 0.65f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.2f, -7.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(9.5f, -0.2f, -7.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, -0.2f, -9.5f);

	glNormal3f(0, -1, 0);
	if (!textureOn) glColor3f(0.45f, 0.45f, 0.45f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 0.0f); glVertex3f(9.5f, -0.8f, -7.5f);
	glTexCoord2f(5.5f, 1.0f); glVertex3f(9.5f, -0.8f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, -0.8f, -9.5f);

	glNormal3f(1, 0, 0);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(9.5f, -0.8f, -7.5f);
	glTexCoord2f(2.0f, 0.0f); glVertex3f(9.5f, -0.8f, -9.5f);
	glTexCoord2f(2.0f, 1.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(9.5f, -0.2f, -7.5f);

	glNormal3f(-1, 0, 0);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.8f, -9.5f);
	glTexCoord2f(2.0f, 0.0f); glVertex3f(4.3f, -0.8f, -7.5f);
	glTexCoord2f(2.0f, 1.0f); glVertex3f(4.3f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, -0.2f, -9.5f);
	glEnd();

	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, redWallTex);
	}
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	if (!textureOn) glColor3f(0.7f, 0.2f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(9.5f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(9.5f, 3.0f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, 3.0f, -7.5f);

	glNormal3f(0.0f, 0.0f, -1.0f);
	if (!textureOn) glColor3f(0.6f, 0.15f, 0.15f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(4.3f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(9.5f, 3.0f, -9.5f);

	glNormal3f(0.0f, 1.0f, 0.0f);
	if (!textureOn) glColor3f(0.75f, 0.25f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, 3.0f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(9.5f, 3.0f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(9.5f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, 3.0f, -9.5f);

	glNormal3f(0.0f, -1.0f, 0.0f);
	if (!textureOn) glColor3f(0.5f, 0.1f, 0.1f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(9.5f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, -0.2f, -7.5f);

	glNormal3f(1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(9.5f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(9.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(9.5f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(9.5f, 3.0f, -7.5f);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(4.3f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(4.3f, 3.0f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, 3.0f, -9.5f);
	glEnd();

	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, roofTex);
	}
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.707f, 0.707f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, 3.0f, -7.0f);
	glTexCoord2f(3.0f, 0.0f); glVertex3f(7.0f, 3.0f, -7.0f);
	glTexCoord2f(3.0f, 1.0f); glVertex3f(8.5f, 3.8f, -8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, 3.8f, -8.5f);

	glNormal3f(0.0f, 0.707f, -0.707f);
	if (!textureOn) glColor3f(0.15f, 0.15f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, 3.0f, -10.0f);
	glTexCoord2f(6.0f, 0.0f); glVertex3f(10.0f, 3.0f, -10.0f);
	glTexCoord2f(6.0f, 1.0f); glVertex3f(8.5f, 3.8f, -8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, 3.8f, -8.5f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//7. front wall (middle)
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor3f(0.15f, 0.15f, 0.15f);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-4.3f, 3.0f, -7.0f);
	glVertex3f(4.3f, 3.0f, -7.0f);
	glVertex3f(4.3f, 3.0f, -10.0f);
	glVertex3f(-4.3f, 3.0f, -10.0f);
	glEnd();

	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, greyWallTex);
	}

	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	if (!textureOn) glColor3f(0.6f, 0.6f, 0.6f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.3f, -0.8f, -7.5f);
	glTexCoord2f(2.5f, 0.0f); glVertex3f(-1.5f, -0.8f, -7.5f);
	glTexCoord2f(2.5f, 1.0f); glVertex3f(-1.5f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.3f, -0.2f, -7.5f);

	glNormal3f(0, 0, -1);
	if (!textureOn) glColor3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.3f, -0.8f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(2.5f, 1.0f); glVertex3f(-1.5f, -0.2f, -9.5f);
	glTexCoord2f(2.5f, 0.0f); glVertex3f(-1.5f, -0.8f, -9.5f);

	glNormal3f(1, 0, 0);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.5f, -0.8f, -7.5f);
	glTexCoord2f(2.0f, 0.0f); glVertex3f(-1.5f, -0.8f, -9.5f);
	glTexCoord2f(2.0f, 1.0f); glVertex3f(-1.5f, -0.2f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.5f, -0.2f, -7.5f);

	glNormal3f(0, 0, 1);
	if (!textureOn) glColor3f(0.6f, 0.6f, 0.6f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.5f, -0.8f, -7.5f);
	glTexCoord2f(2.5f, 0.0f); glVertex3f(4.3f, -0.8f, -7.5f);
	glTexCoord2f(2.5f, 1.0f); glVertex3f(4.3f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.5f, -0.2f, -7.5f);

	glNormal3f(0, 0, -1);
	if (!textureOn) glColor3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.5f, -0.8f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.5f, -0.2f, -9.5f);
	glTexCoord2f(2.5f, 1.0f); glVertex3f(4.3f, -0.2f, -9.5f);
	glTexCoord2f(2.5f, 0.0f); glVertex3f(4.3f, -0.8f, -9.5f);

	glNormal3f(-1, 0, 0);
	if (!textureOn) glColor3f(0.55f, 0.55f, 0.55f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.5f, -0.8f, -9.5f);
	glTexCoord2f(2.0f, 0.0f); glVertex3f(1.5f, -0.8f, -7.5f);
	glTexCoord2f(2.0f, 1.0f); glVertex3f(1.5f, -0.2f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.5f, -0.2f, -9.5f);
	glEnd();

	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, redWallTex);
	}
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	if (!textureOn) glColor3f(0.7f, 0.2f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.3f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.5f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.5f, 3.0f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.3f, 3.0f, -7.5f);

	glNormal3f(0, 0, -1);
	if (!textureOn) glColor3f(0.6f, 0.15f, 0.15f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.3f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.5f, 3.0f, -9.5f);

	glNormal3f(1, 0, 0);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.5f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.5f, 1.5f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.5f, 1.5f, -7.5f);

	glNormal3f(0, 0, 1);
	if (!textureOn) glColor3f(0.7f, 0.2f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.5f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(4.3f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(4.3f, 3.0f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.5f, 3.0f, -7.5f);

	glNormal3f(0, 0, -1);
	if (!textureOn) glColor3f(0.6f, 0.15f, 0.15f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(1.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.5f, 3.0f, -9.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, 3.0f, -9.5f);

	glNormal3f(-1, 0, 0);
	if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.5f, -0.2f, -9.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(1.5f, -0.2f, -7.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.5f, 1.5f, -7.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.5f, 1.5f, -9.5f);
	glEnd();

	//front wall (curve wall)
	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, redWallTex);
	}
	glBegin(GL_QUADS);

	for (int i = 0; i < numSegments; i++) {
		float theta1 = PI - (i * PI / numSegments);
		float theta2 = PI - ((i + 1) * PI / numSegments);

		float x1 = radiusX * cos(theta1);
		float y1 = baseY + radiusY * sin(theta1);
		float x2 = radiusX * cos(theta2);
		float y2 = baseY + radiusY * sin(theta2);

		float u1 = (x1 + 1.5f) / 3.0f;
		float u2 = (x2 + 1.5f) / 3.0f;

		// front
		glNormal3f(0.0f, 0.0f, 1.0f);
		if (!textureOn) glColor3f(0.7f, 0.2f, 0.2f);
		glTexCoord2f(u1, 1.0f); glVertex3f(x1, 3.0f, -7.5f); //top left
		glTexCoord2f(u1, 0.0f); glVertex3f(x1, y1, -7.5f);   //bottom left
		glTexCoord2f(u2, 0.0f); glVertex3f(x2, y2, -7.5f);   //bottom right
		glTexCoord2f(u2, 1.0f); glVertex3f(x2, 3.0f, -7.5f); //top right

		// back
		glNormal3f(0.0f, 0.0f, -1.0f);
		if (!textureOn) glColor3f(0.6f, 0.15f, 0.15f);
		glTexCoord2f(u2, 1.0f); glVertex3f(x2, 3.0f, -9.5f);
		glTexCoord2f(u2, 0.0f); glVertex3f(x2, y2, -9.5f);
		glTexCoord2f(u1, 0.0f); glVertex3f(x1, y1, -9.5f);
		glTexCoord2f(u1, 1.0f); glVertex3f(x1, 3.0f, -9.5f);

		// inner curve (connecting front and back)
		glNormal3f(-cos(theta1), -sin(theta1), 0.0f);
		if (!textureOn) glColor3f(0.65f, 0.18f, 0.18f);
		glTexCoord2f(u1, 0.0f); glVertex3f(x1, y1, -7.5f);
		glTexCoord2f(u1, 1.0f); glVertex3f(x1, y1, -9.5f);

		glNormal3f(-cos(theta2), -sin(theta2), 0.0f);
		glTexCoord2f(u2, 1.0f); glVertex3f(x2, y2, -9.5f);
		glTexCoord2f(u2, 0.0f); glVertex3f(x2, y2, -7.5f);
	}
	glEnd();

	// connect to roof
	if (textureOn) {
		glBindTexture(GL_TEXTURE_2D, roofTex);
	}
	glBegin(GL_QUADS);
	// inside roof
	glNormal3f(0.0f, 0.707f, 0.707f);
	if (!textureOn) glColor3f(0.2f, 0.2f, 0.25f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.3f, 3.0f, -7.0f);
	glTexCoord2f(4.0f, 0.0f); glVertex3f(4.3f, 3.0f, -7.0f);
	glTexCoord2f(4.0f, 1.0f); glVertex3f(4.3f, 3.8f, -8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.3f, 3.8f, -8.5f);

	// outside roof
	glNormal3f(0.0f, 0.707f, -0.707f);
	if (!textureOn) glColor3f(0.15f, 0.15f, 0.2f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(4.3f, 3.0f, -10.0f);
	glTexCoord2f(4.0f, 0.0f); glVertex3f(-4.3f, 3.0f, -10.0f);
	glTexCoord2f(4.0f, 1.0f); glVertex3f(-4.3f, 3.8f, -8.5f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(4.3f, 3.8f, -8.5f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();

	glPopMatrix();

	//garden table and chair
	gardenTable(-6, -0.8, -3.5);

	//house
	glPushMatrix();
	glTranslatef(0.0f, 1.2f, -8.5f);
	house();
	glPopMatrix();

	//pathway
	if (textureOn) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, pathWayTex);
		glColor3f(1.0f, 1.0f, 1.0f);
	}
	else {
		glDisable(GL_TEXTURE_2D);
	}

	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);
	if (!textureOn) glColor3f(0.6f, 0.6f, 0.65f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.5f, -0.79f, -8.5f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(1.5f, -0.79f, -8.5f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.5f, -0.79f, 15.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.5f, -0.79f, 15.0f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}

void weapon() {
	GLuint handleTexture = 0;
	GLuint bladeTexture = 0;
	GLuint jointTexture = 0;

	//Spear Blade
	glPushMatrix();
	glTranslatef(0, 4, 0.1);

	//Lower Blade
	bladeTexture = loadTexture("ice+and+snow+ground-64x64.bmp");
	glColor3f(1, 1, 1);
	glPushMatrix();
	glRotatef(180, 1, 0, 0);
	glScalef(1.5, 1.5, 0.2);
	pyramid();
	glDeleteTextures(1, &bladeTexture);
	glPopMatrix();

	//Upper Blade
	bladeTexture = loadTexture("ice+and+snow+ground-64x64.bmp");
	glColor3f(1, 1, 1);
	glPushMatrix();
	glScalef(1.5, 3.25, 0.2);
	pyramid();
	glDeleteTextures(1, &bladeTexture);
	glPopMatrix();
	glPopMatrix();

	//Ring blade between
	glPushMatrix();
	glTranslatef(0, attackRing, 0);
	glColor3f(1, 1, 1);
	glPushMatrix();
	glScalef(0.7, 0.7, 0.7);
	glTranslatef(0, 2, 0);
	glPushMatrix();
	glTranslatef(0, 0, 0.1);
	bladeTexture = loadTexture("metallic+stripes-64x64.bmp");
	glColor3f(0, 0, 1);
	disk(1.35, 1.45);
	glDeleteTextures(1, &bladeTexture);
	glPopMatrix();
	bladeTexture = loadTexture("metallic+stripes-64x64.bmp");
	glColor3f(0, 0, 1);
	cylinder(1.45, 1.45, 0.2);
	glDeleteTextures(1, &bladeTexture);
	glPushMatrix();
	glTranslatef(0, 0, 0.1);
	bladeTexture = loadTexture("ice+and+snow+ground-64x64.bmp");
	glColor3f(1, 1, 1);
	disk(1.45, 1.65);
	glDeleteTextures(1, &bladeTexture);
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	//Handle
	handleTexture = loadTexture("metallic+stripes-64x64.bmp");
	glColor3f(1, 1, 1);
	glPushMatrix();
	glTranslatef(0, 3, 0.1);
	glPushMatrix();
	glRotatef(-90, 0, 0, 1);
	glRotatef(90, 0, 1, 0);
	cylinder(0.15, 0.15, 12);
	glPopMatrix();
	glDeleteTextures(1, &handleTexture);

	handleTexture = loadTexture("rose-gold-leather-textured-background.bmp");
	glPopMatrix();
	glColor3f(0.65, 0.35, 0);
	glPushMatrix();
	glTranslatef(0, 3, 0.1);
	glRotatef(-90, 0, 0, 1);
	glRotatef(90, 0, 1, 0);
	cylinder(0.2, 0.2, 3);
	glPopMatrix();
	glDeleteTextures(1, &handleTexture);
	

	//Lower Handle Cover
	glColor3f(1, 1, 1);
	glPushMatrix();
	glTranslatef(0, -9, 0.1);
	jointTexture = loadTexture("metallic+stripes-64x64.bmp");
	glColor3f(0.8, 0.8, 0.8);
	sphere(0.2);
	glDeleteTextures(1, &jointTexture);
	glPushMatrix();
	glRotatef(-90, 0, 0, 1);
	glRotatef(90, 0, 1, 0);
	bladeTexture = loadTexture("ice+and+snow+ground-64x64.bmp");
	cone(0.15, 1);
	glDeleteTextures(1, &bladeTexture);
	glPopMatrix();
	glPopMatrix();

	//Upper Handle Cover
	glPushMatrix();
	glTranslatef(0, 3, 0.1);
	jointTexture = loadTexture("metallic+stripes-64x64.bmp");
	glColor3f(0.8, 0.8, 0.8);
	sphere(0.35);
	glDeleteTextures(1, &jointTexture);

	// Left Blade
	glPushMatrix();
	glRotatef(90, 0, 1, 0);
	jointTexture = loadTexture("metallic+stripes-64x64.bmp");
	glColor3f(0, 1, 1);
	cylinder(0.15, 0.15, 1);
	glDeleteTextures(1, &jointTexture);
	
	glPushMatrix();
	glTranslatef(0, 0, 1);
	jointTexture = loadTexture("metallic+stripes-64x64.bmp");
	glColor3f(0.8, 0.8, 0.8);
	sphere(0.15);
	glDeleteTextures(1, &jointTexture);

	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	glTranslatef(0, -0.4, 0);
	glScalef(0.1, 0.8, 0.5);
	bladeTexture = loadTexture("ice+and+snow+ground-64x64.bmp");
	pyramid();
	glDeleteTextures(1, &bladeTexture);
	glPopMatrix();

	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glTranslatef(0, 0.4, 0);
	glScalef(0.1, 1.25, 0.5);
	bladeTexture = loadTexture("ice+and+snow+ground-64x64.bmp");
	pyramid();
	glDeleteTextures(1, &bladeTexture);
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	//Right Blade
	glPushMatrix();
	glRotatef(-90, 0, 1, 0);
	jointTexture = loadTexture("metallic+stripes-64x64.bmp");
	glColor3f(0, 1, 1);
	cylinder(0.15, 0.15, 1);
	glDeleteTextures(1, &jointTexture);
	glPushMatrix();
	glTranslatef(0, 0, 1);
	glColor3f(0.8, 0.8, 0.8);
	jointTexture = loadTexture("metallic+stripes-64x64.bmp");
	sphere(0.15);
	glDeleteTextures(1, &jointTexture);

	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	glTranslatef(0, -0.4, 0);
	glScalef(0.1, 0.8, 0.5);
	bladeTexture = loadTexture("ice+and+snow+ground-64x64.bmp");
	pyramid();
	glDeleteTextures(1, &bladeTexture);
	glPopMatrix();

	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glTranslatef(0, 0.4, 0);
	glScalef(0.1, 1.25, 0.5);
	bladeTexture = loadTexture("ice+and+snow+ground-64x64.bmp");
	pyramid();
	glDeleteTextures(1, &bladeTexture);
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	glPopMatrix();
}

void characterArm(int handIndex) {
	GLuint textureArr[2];
	GLUquadricObj* quadric = gluNewQuadric();
	gluQuadricTexture(quadric, GL_TRUE);
	gluQuadricDrawStyle(quadric, GLU_FILL);
	float curl = (handIndex == 0) ? 1.0f : -1.0f;

	glPushMatrix();
		// shoulder joint
		glTranslatef(0.105, 0.105, 0);
		glRotatef(shoulderAngles[handIndex], 1.0f, 0.0f, 0.0f);
		textureArr[0] = loadTexture("cloth+fiber+fabric-512x512.bmp");
		glColor3f(0.9, 0.9, 0.95);
		gluSphere(quadric, 0.15, 20, 20);
		glRotatef(90.0, 1.0, 0.0, 0.0);

		// upper arm
		glColor3f(0.9, 0.9, 0.95);
		gluCylinder(quadric, 0.15, 0.13, 0.6, 30, 30);

		// elbow joint
		glTranslatef(0.0, 0.0, 0.6);
		glColor3f(0.15, 0.15, 0.15);
		gluSphere(quadric, 0.12, 20, 20);
		glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(forearmAngles[handIndex], 1.0f, 0.0f, 0.0f);
		
		// forearm
		glColor3f(0.15, 0.15, 0.15);
		gluCylinder(quadric, 0.13, 0.10, 0.5, 30, 30);
		glTranslatef(0.0, 0.0, 0.5);
		glDeleteTextures(1, &textureArr[0]);
		
		//hand palm
		glRotatef(palmAngles[handIndex], 0.0f, 0.0f, 1.0f);
		
		// attack palm
		if (isAttack && handIndex == 1)
		{
			glRotatef(attackPalmAngles, 0, 1, 0);
		}

		glColor3f(0.15, 0.15, 0.15);
		gluSphere(quadric, 0.10, 30, 30);

		glTranslatef(0.0, 0, 0.15);
		glColor3f(0.15, 0.15, 0.15);

		glPushMatrix();
			glScalef(1.15, 0.7, 1.1);
			gluSphere(quadric, 0.10, 30, 30);
		glPopMatrix(); 

	
		glPushMatrix();
			// move the weapon to the palm center
			glTranslatef(0.2, 0.0,0.05);
			// rotate weapon
			glRotatef(90, 1.0, 0.0, 0.0);
			glRotatef(-90, 0.0, 0.0, 1.0);
			glScalef(0.2, 0.2, 0.2);

			glTranslatef(0.0, 2.0, 0.0);
			if (handIndex == 1) {
				glPushMatrix();
				glScalef(1, 1.5, 1);
				glTranslatef(0, attackWeaponSpear, 0);
				weapon();
				glPopMatrix();
			}
		glPopMatrix();

		
		glTranslatef(0.0, 0.015, 0.08);

		//--------------------
		//finger

		// index finger (Moved from -0.06 to 0.06)
		glPushMatrix();
			glTranslatef(0.06, 0.0, -0.01);
			glRotatef(fingerAngles[handIndex][1][0], 1.0f, 0.0f, 0.0f); //base
			glColor3f(0.15, 0.15, 0.15);
			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.02);
				drawBox(0.018, 0.018, 0.02);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.04);
			glRotatef(fingerAngles[handIndex][1][1], 1.0f, 0.0f, 0.0f); // mid
			glColor3f(0.95, 0.8, 0.7);

			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.02);
				drawBox(0.015, 0.015, 0.02);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.035);
			glRotatef(fingerAngles[handIndex][1][2], 1.0f, 0.0f, 0.0f); // tip

			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.02);
				drawBox(0.015, 0.015, 0.02);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.035);
			gluCylinder(quadric, 0.015, 0.012, 0.03, 15, 1);
			glTranslatef(0.0, 0.0, 0.03);
			gluDisk(quadric, 0.0, 0.012, 15, 1);
		glPopMatrix();

		// middle finger
		glPushMatrix();
			glTranslatef(0.02, 0.0, 0.01);
			glRotatef(fingerAngles[handIndex][2][0], 1.0f, 0.0f, 0.0f); //base
			glColor3f(0.15, 0.15, 0.15);
			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.02);
				drawBox(0.018, 0.018, 0.02);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.04);
			glRotatef(fingerAngles[handIndex][2][1], 1.0f, 0.0f, 0.0f); // mid
			glColor3f(0.95, 0.8, 0.7);

			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.025);
				drawBox(0.015, 0.015, 0.025);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.045);
			glRotatef(fingerAngles[handIndex][2][2], 1.0f, 0.0f, 0.0f); // tip

			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.025);
				drawBox(0.015, 0.015, 0.025);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.045);
			gluCylinder(quadric, 0.015, 0.012, 0.03, 15, 1);
			glTranslatef(0.0, 0.0, 0.03);
			gluDisk(quadric, 0.0, 0.012, 15, 1);
		glPopMatrix();

		// ring finger 
		glPushMatrix();
			glTranslatef(-0.02, 0.0, 0.0);
			glRotatef(fingerAngles[handIndex][3][0], 1.0f, 0.0f, 0.0f); //base
			glColor3f(0.15, 0.15, 0.15);
			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.02);
				drawBox(0.018, 0.018, 0.02);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.04);
			glRotatef(fingerAngles[handIndex][3][1], 1.0f, 0.0f, 0.0f); // mid
			glColor3f(0.95, 0.8, 0.7);

			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.0225);
				drawBox(0.015, 0.015, 0.0225);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.04);
			glRotatef(fingerAngles[handIndex][3][2], 1.0f, 0.0f, 0.0f); // tip

			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.0225);
				drawBox(0.015, 0.015, 0.0225);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.04);
			gluCylinder(quadric, 0.015, 0.012, 0.03, 15, 1);
			glTranslatef(0.0, 0.0, 0.03);
			gluDisk(quadric, 0.0, 0.012, 15, 1);
		glPopMatrix();

		// pinky finger 
		glPushMatrix();
			glTranslatef(-0.06, 0.0, -0.02);
			glRotatef(fingerAngles[handIndex][4][0], 1.0f, 0.0f, 0.0f); //base
			glColor3f(0.15, 0.15, 0.15);
			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.0175);
				drawBox(0.015, 0.015, 0.0175);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.035);
			glRotatef(fingerAngles[handIndex][4][1], 1.0f, 0.0f, 0.0f); // mid
			glColor3f(0.95, 0.8, 0.7);

			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.01625);
				drawBox(0.012, 0.012, 0.01625);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.0275);
			glRotatef(fingerAngles[handIndex][4][2], 1.0f, 0.0f, 0.0f); // tip


			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.01625);
				drawBox(0.012, 0.012, 0.01625);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.0275);
			gluCylinder(quadric, 0.012, 0.009, 0.025, 15, 1);
			glTranslatef(0.0, 0.0, 0.025);
			gluDisk(quadric, 0.0, 0.009, 15, 1);
		glPopMatrix();

		// thumb 
		glPushMatrix();
			glTranslatef(0.09, 0.0, -0.08);
			glRotatef(fingerAngles[handIndex][0][0], 1.0f, 0.0f, 0.0f); //base
			glRotatef(35.0, 0.0, 1.0, 0.0); 
			glRotatef(20.0, 1.0, 0.0, 0.0);
			glColor3f(0.15, 0.15, 0.15);
			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.015);
				drawBox(0.02, 0.02, 0.015);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.03);
			glRotatef(fingerAngles[handIndex][0][1], 1.0f, 0.0f, 0.0f); // mid
			glColor3f(0.95, 0.8, 0.7);

			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.015);
				drawBox(0.018, 0.018, 0.015);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.025);
			glRotatef(-fingerAngles[handIndex][0][2], 1.0f, 0.0f, 0.0f); // tip

			glPushMatrix();
				glTranslatef(0.0, 0.0, 0.015);
				drawBox(0.018, 0.018, 0.015);
			glPopMatrix();

			glTranslatef(0.0, 0.0, 0.025);
			gluCylinder(quadric, 0.018, 0.014, 0.03, 15, 1);
			glTranslatef(0.0, 0.0, 0.03);
			gluDisk(quadric, 0.0, 0.014, 15, 1);
		glPopMatrix();
	glPopMatrix();
	gluDeleteQuadric(quadric);
}

void face() {
	// 1. 
	glBegin(GL_QUADS);
	// Bottom
	glVertex3f(-0.15, 0.6, 0);
	glVertex3f(0.15, 0.6, 0);
	glVertex3f(0.15, 0.6, -0.45);
	glVertex3f(-0.15, 0.6, -0.45);

	// Right
	glVertex3f(0.15, 0.6, 0.0);
	glVertex3f(0.15, 0.6, -0.50);
	glVertex3f(0.10, 0.7, -0.40);
	glVertex3f(0.10, 0.7, -0.05);

	// Left
	glVertex3f(-0.15, 0.6, 0.0);
	glVertex3f(-0.15, 0.6, -0.50);
	glVertex3f(-0.10, 0.7, -0.40);
	glVertex3f(-0.10, 0.7, -0.05);

	// Top
	glVertex3f(-0.10, 0.7, -0.05);
	glVertex3f(0.10, 0.7, -0.05);
	glVertex3f(0.10, 0.7, -0.40);
	glVertex3f(-0.10, 0.7, -0.40);

	// Front 
	glVertex3f(-0.15, 0.6, 0.0);
	glVertex3f(0.15, 0.6, 0.0);
	glVertex3f(0.10, 0.7, -0.05);
	glVertex3f(-0.10, 0.7, -0.05);

	// Back
	glVertex3f(-0.15, 0.6, -0.45);
	glVertex3f(0.15, 0.6, -0.45);
	glVertex3f(0.10, 0.7, -0.40);
	glVertex3f(-0.10, 0.7, -0.40);

	glEnd();

	// 2.
	glBegin(GL_QUADS);
	// bottom
	glVertex3f(-0.2, 0.5, 0.05);
	glVertex3f(0.2, 0.5, 0.05);
	glVertex3f(0.2, 0.5, -0.45);
	glVertex3f(-0.2, 0.5, -0.45);

	// right
	glVertex3f(0.2, 0.5, 0.05);
	glVertex3f(0.2, 0.5, -0.45);
	glVertex3f(0.15, 0.6, -0.5);
	glVertex3f(0.15, 0.6, 0.0);

	// left
	glVertex3f(-0.2, 0.5, 0.05);
	glVertex3f(-0.2, 0.5, -0.45);
	glVertex3f(-0.15, 0.6, -0.5);
	glVertex3f(-0.15, 0.6, 0.0);

	// top
	glVertex3f(-0.15, 0.6, 0);
	glVertex3f(0.15, 0.6, 0);
	glVertex3f(0.15, 0.6, -0.45);
	glVertex3f(-0.15, 0.6, -0.45);

	// front 
	glVertex3f(-0.2, 0.5, 0.05);
	glVertex3f(0.2, 0.5, 0.05);
	glVertex3f(0.15, 0.6, 0);
	glVertex3f(-0.15, 0.6, 0);


	// back
	glVertex3f(-0.2, 0.5, -0.45);
	glVertex3f(0.2, 0.5, -0.45);
	glVertex3f(0.15, 0.6, -0.5);
	glVertex3f(-0.15, 0.6, -0.5);

	glEnd();


	// 3.
	glBegin(GL_QUADS);
	// top
	glVertex3f(-0.2, 0.5, 0.05);
	glVertex3f(0.2, 0.5, 0.05);
	glVertex3f(0.2, 0.5, -0.45);
	glVertex3f(-0.2, 0.5, -0.45);

	// front
	glVertex3f(-0.2, 0.4, 0.03);
	glVertex3f(0.2, 0.4, 0.03);
	glVertex3f(0.2, 0.5, 0.05);
	glVertex3f(-0.2, 0.5, 0.05);

	// right
	glVertex3f(0.2, 0.4, 0.03);
	glVertex3f(0.2, 0.4, -0.4);
	glVertex3f(0.2, 0.5, -0.45);
	glVertex3f(0.2, 0.5, 0.05);

	// bottom
	glVertex3f(-0.2, 0.4, 0.03);
	glVertex3f(0.2, 0.4, 0.03);
	glVertex3f(0.2, 0.4, -0.40);
	glVertex3f(-0.2, 0.4, -0.40);

	// left
	glVertex3f(-0.2, 0.4, 0.03);
	glVertex3f(-0.2, 0.4, -0.4);
	glVertex3f(-0.2, 0.5, -0.45);
	glVertex3f(-0.2, 0.5, 0.05);

	// back
	glVertex3f(-0.2, 0.4, -0.4);
	glVertex3f(0.2, 0.4, -0.4);
	glVertex3f(0.2, 0.5, -0.45);
	glVertex3f(-0.2, 0.5, -0.45);

	glEnd();

	// 4.
	glBegin(GL_QUADS);

	// top
	glVertex3f(-0.2, 0.4, 0.03);
	glVertex3f(0.2, 0.4, 0.03);
	glVertex3f(0.2, 0.4, -0.40);
	glVertex3f(-0.2, 0.4, -0.40);

	// front
	glVertex3f(-0.15, 0.3, 0.05);
	glVertex3f(0.15, 0.3, 0.05);
	glVertex3f(0.2, 0.4, 0.03);
	glVertex3f(-0.2, 0.4, 0.03);

	// right
	glVertex3f(0.15, 0.3, 0.05);
	glVertex3f(0.15, 0.3, -0.35);
	glVertex3f(0.2, 0.4, -0.40);
	glVertex3f(0.2, 0.4, 0.03);

	// bottom
	glVertex3f(-0.15, 0.3, 0.05);
	glVertex3f(0.15, 0.3, 0.05);
	glVertex3f(0.15, 0.3, -0.35);
	glVertex3f(-0.15, 0.3, -0.35);

	// left
	glVertex3f(-0.15, 0.3, -0.35);
	glVertex3f(-0.15, 0.3, 0.05);
	glVertex3f(-0.2, 0.4, 0.03);
	glVertex3f(-0.2, 0.4, -0.4);

	// back
	glVertex3f(-0.15, 0.3, -0.35);
	glVertex3f(0.15, 0.3, -0.35);
	glVertex3f(0.2, 0.4, -0.40);
	glVertex3f(-0.2, 0.4, -0.4);

	glEnd();

	// 5.
	glBegin(GL_QUADS);

	// top
	glVertex3f(-0.15, 0.3, 0.05);
	glVertex3f(0.15, 0.3, 0.03);
	glVertex3f(0.15, 0.3, -0.35);
	glVertex3f(-0.15, 0.3, -0.35);

	// front
	glVertex3f(-0.10, 0.2, 0.02);
	glVertex3f(0.10, 0.2, 0.02);
	glVertex3f(0.15, 0.3, 0.05);
	glVertex3f(-0.15, 0.3, 0.05);

	// right
	glVertex3f(0.1, 0.2, 0.02);
	glVertex3f(0.1, 0.2, -0.30);
	glVertex3f(0.15, 0.3, -0.35);
	glVertex3f(0.15, 0.3, 0.05);

	// bottom
	glVertex3f(-0.1, 0.2, 0.02);
	glVertex3f(0.1, 0.2, 0.02);
	glVertex3f(0.1, 0.2, -0.30);
	glVertex3f(-0.1, 0.2, -0.30);

	// left
	glVertex3f(-0.1, 0.2, 0.02);
	glVertex3f(-0.1, 0.2, -0.30);
	glVertex3f(-0.15, 0.3, -0.35);
	glVertex3f(-0.15, 0.3, 0.05);

	// back
	glVertex3f(-0.1, 0.2, -0.3);
	glVertex3f(0.1, 0.2, -0.30);
	glVertex3f(0.15, 0.3, -0.35);
	glVertex3f(-0.15, 0.3, -0.35);

	glEnd();

	// 6.
	glBegin(GL_QUADS);

	// top
	glVertex3f(-0.1, 0.2, 0.02);
	glVertex3f(0.1, 0.2, 0.02);
	glVertex3f(0.1, 0.2, -0.30);
	glVertex3f(-0.1, 0.2, -0.30);

	// front
	glVertex3f(-0.05, 0.18, 0.01);
	glVertex3f(0.05, 0.18, 0.01);
	glVertex3f(0.1, 0.2, 0.02);
	glVertex3f(-0.1, 0.2, 0.02);

	// right
	glVertex3f(0.05, 0.18, 0.01);
	glVertex3f(0.1, 0.2, -0.3);
	glVertex3f(0.10, 0.2, -0.3);
	glVertex3f(0.10, 0.2, 0.02);

	// bottom
	glVertex3f(-0.05, 0.18, 0.01);
	glVertex3f(0.05, 0.18, 0.01);
	glVertex3f(0.1, 0.2, -0.3);
	glVertex3f(-0.1, 0.2, -0.3);

	// left
	glVertex3f(-0.05, 0.18, 0.01);
	glVertex3f(-0.1, 0.2, -0.3);
	glVertex3f(-0.10, 0.2, -0.3);
	glVertex3f(-0.10, 0.2, 0.02);



	glEnd();


}

void hat() {
	GLUquadricObj* quadric = gluNewQuadric();

	glPushMatrix();

	glTranslatef(0.0, 0.16, 0.0);
	glRotatef(10.0, 1.0, 0.0, 0.0);
	glRotatef(-90.0, 1.0, 0.0, 0.0);

	// cone
	glColor3f(0.0, 0.0, 0.0);
	gluCylinder(quadric, 0.45, 0.0, 0.35, 30, 5);

	// red ribbon
	glColor3f(0.8, 0.1, 0.1);
	gluCylinder(quadric, 0.455, 0.39, 0.05, 30, 1);

	glPushMatrix();
	glColor3f(0.9, 0.8, 0.9);
	glTranslatef(0, 0, 0.2);
	gluCylinder(quadric, 0.2, 0.15, 0.05, 30, 1);
	glPopMatrix();


	// bottom
	glColor3f(0, 0, 0);
	glRotatef(180.0, 1.0, 0.0, 0.0);
	gluDisk(quadric, 0.0, 0.45, 30, 1);

	glPopMatrix();
	gluDeleteQuadric(quadric);
}

void hair() {
	GLUquadricObj* quadric = gluNewQuadric();

	glColor3f(0.15, 0.12, 0.10);

	glPushMatrix();
	glTranslatef(0.0, 0.07, -0.13);

	glScalef(0.85, 1.0, 0.95);

	gluSphere(quadric, 0.28, 20, 20);

	glPopMatrix();
	gluDeleteQuadric(quadric);
}

void character() {
	GLuint textureArr[2];
	GLUquadricObj* quadric = gluNewQuadric();
	gluQuadricTexture(quadric, GL_TRUE);
	gluQuadricDrawStyle(quadric, GLU_FILL);

	glPushMatrix();

	// upper body
	textureArr[0] = loadTexture("circles+pattern+clothing-256x256.bmp");
	glColor3f(0.9, 0.9, 0.95);
	glPushMatrix();
	glTranslatef(0.0, 0.0, 0.0);
	glScalef(1.0, 1.0, 0.65);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	gluCylinder(quadric, 0.45, 0.605, 0.6, 30, 10);
	glPopMatrix();


	// collar
	glColor3f(0.9, 0.9, 0.95);
	glPushMatrix();
	glTranslatef(0.0, 0.6, 0.0);
	glScalef(1.0, 0.35, 0.65);
	gluSphere(quadric, 0.60, 30, 30);
	glPopMatrix();

	// lower body
	glColor3f(0.15, 0.15, 0.15);
	glPushMatrix();
	glTranslatef(0.0, -0.4, 0.0);
	glScalef(1.0, 1.0, 0.65);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	gluCylinder(quadric, 0.43, 0.45, 0.4, 30, 10);
	glDeleteTextures(1, &textureArr[0]);
	glPopMatrix();

	//------------------------------------------
	//head

	glPushMatrix();
	glTranslatef(0.0, 1.15, 0.0);

	glRotatef(headAngle, 0.0f, 1.0f, 0.0f);

	//------------------------------------------
	//hair and hat
	glPushMatrix();
	hair();
	hat();
	glPopMatrix();

	//------------------------------------------
	//neck
	glPushMatrix();

	glTranslatef(0.0, -0.5, 0.0);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	glColor3f(0.95, 0.8, 0.7);

	gluCylinder(quadric, 0.15, 0.12, 0.3, 20, 20);

	glPopMatrix();

	//------------------------------------------
	//face
	glPushMatrix();
	glColor3f(0.95, 0.8, 0.7);
	glTranslatef(0, -0.45, 0.2);
	face();
	glPopMatrix();

	//------------------------------------------
	//ears
	//left ears
	glPushMatrix();
	glTranslatef(-0.2, -0.05, 0.0);
	glRotatef(100.0, 0.0, 1.0, 0.0);
	glRotatef(-15, 0, 0, 1);
	glRotatef(-20, 1, 0, 0);


	glColor3f(0.95, 0.8, 0.7);
	glScalef(0.15, 0.55, 0.35);
	gluSphere(quadric, 0.1, 15, 15);
	glPopMatrix();


	//right ears
	glPushMatrix();
	glTranslatef(0.2, -0.05, 0.0);
	glRotatef(-100.0, 0.0, 1.0, 0.0);
	glRotatef(15, 0, 0, 1);
	glRotatef(-20, 1, 0, 0);


	glColor3f(0.95, 0.8, 0.7);
	glScalef(0.15, 0.55, 0.35);
	gluSphere(quadric, 0.1, 15, 15);
	glPopMatrix();

	//------------------------------------------
	//nose
	glPushMatrix();
	glColor3f(0.95, 0.8, 0.7);
	glTranslatef(0, 0, -0.13);
	glPushMatrix();
	glBegin(GL_TRIANGLES);

	//right
	glVertex3f(0.01, -0.15, 0.42);
	glVertex3f(0.037, -0.17, 0.37);
	glVertex3f(0.01, -0.06, 0.37);

	//left
	glVertex3f(-0.01, -0.15, 0.42);
	glVertex3f(-0.037, -0.17, 0.37);
	glVertex3f(-0.01, -0.06, 0.37);

	glEnd();

	glBegin(GL_QUADS);
	//middle
	glVertex3f(-0.01, -0.15, 0.42);
	glVertex3f(0.01, -0.15, 0.42);
	glVertex3f(0.01, -0.06, 0.37);
	glVertex3f(-0.01, -0.06, 0.37);


	//bottom
	glVertex3f(-0.037, -0.17, 0.37);
	glVertex3f(0.037, -0.17, 0.37);
	glVertex3f(0.01, -0.15, 0.42);
	glVertex3f(-0.01, -0.15, 0.42);

	glEnd();



	glPopMatrix();

	//left hole
	glPushMatrix();
	glColor3f(0, 0, 0);
	glTranslatef(-0.015, -0.159, 0.4);
	glRotatef(60, 1, 0, 0);
	gluDisk(quadric, 0.0, 0.008, 15, 15);
	glPopMatrix();

	//right hole
	glPushMatrix();
	glColor3f(0, 0, 0);
	glTranslatef(0.015, -0.159, 0.4);
	glRotatef(60, 1, 0, 0);
	gluDisk(quadric, 0.0, 0.008, 15, 15);
	glPopMatrix();
	glPopMatrix();

	//------------------------------------------
	//eyes
	//left eye
	glPushMatrix();
	glScalef(1, 0.7, 1);
	glTranslatef(-0.09, -0.02, 0.19);
	glColor3f(0.95, 0.95, 0.95);
	gluSphere(quadric, 0.065, 30, 30);

	glColor3f(0.1, 0.1, 0.1);
	glPushMatrix();
	glTranslatef(0, 0.0, 0.065);
	glScalef(1.0, 1.0, 0.2);
	gluSphere(quadric, 0.012, 30, 30);
	glPopMatrix();
	glPopMatrix();

	//right eye
	glPushMatrix();
	glScalef(1, 0.7, 1);
	glTranslatef(0.09, -0.02, 0.19);
	glColor3f(0.95, 0.95, 0.95);
	gluSphere(quadric, 0.065, 30, 30);

	glColor3f(0.1, 0.1, 0.1);
	glPushMatrix();
	glTranslatef(0.0, 0.0, 0.065);
	glScalef(1.0, 1.0, 0.2);
	gluSphere(quadric, 0.012, 30, 30);
	glPopMatrix();
	glPopMatrix();

	//------------------------------------------
	//mouth 




	glPopMatrix();

	//arm
	glColor3f(0.9, 0.9, 0.95);
	//left arm
	glPushMatrix();
	glTranslatef(-0.45, 0.43, 0.0);
	glRotatef(-15.0, 0.0, 0.0, 1.0);
	glRotatef(180, 0, 1, 0);
	characterArm(0);
	glPopMatrix();

	//right arm
	glPushMatrix();
	glTranslatef(0.45, 0.43, 0.0);
	glRotatef(15.0, 0.0, 0.0, 1.0);
	glScalef(-1.0, 1.0, 1.0);
	glRotatef(180, 0, 1, 0);

	characterArm(1);
	glPopMatrix();

	glPopMatrix();

	gluDeleteQuadric(quadric);
}

void foot() {
	glColor3f(0, 0, 0);
	glBegin(GL_QUADS);
		// Up
		glVertex3f(-0.5, -7, 0);
		glVertex3f(-1.25, -7, 0);
		glVertex3f(-1.25, -7, -1.5);
		glVertex3f(-0.5, -7, -1.5);

		// Front
		glVertex3f(-0.5, -7, -1.5);
		glVertex3f(-1.25, -7, -1.5);
		glVertex3f(-1.25, -7.75, -1.5);
		glVertex3f(-0.5, -7.75, -1.5);

		// Right
		glVertex3f(-0.5, -7.75, -1.5);
		glVertex3f(-0.5, -7.75, 0);
		glVertex3f(-0.5, -7, 0);
		glVertex3f(-0.5, -7, -1.5);

		// Bottom
		glVertex3f(-0.5, -7.75, 0);
		glVertex3f(-1.25, -7.75, 0);
		glVertex3f(-1.25, -7.75, -1.5);
		glVertex3f(-0.5, -7.75, -1.5);

		// Left
		glVertex3f(-1.25, -7.75, 0);
		glVertex3f(-1.25, -7, 0);
		glVertex3f(-1.25, -7, -1.5);
		glVertex3f(-1.25, -7.75, -1.5);

		// Back
		glVertex3f(-1.25, -7, -1.5);
		glVertex3f(-0.5, -7, -1.5);
		glVertex3f(-0.5, -7.75, -1.5);
		glVertex3f(-1.25, -7.75, -1.5);
	glEnd();
	glPushMatrix();
	glTranslatef(-0.85, -7.4, -1.4);
	glScalef(1, 1, 4);
	sphere(0.36);
	glPopMatrix();
}

void leg() {
	GLuint clothTexture = 0;
	// Left Leg
	glPushMatrix();

	clothTexture = loadTexture("brown+fabric-64x64.bmp");
	glBegin(GL_QUADS);
	glColor3f(0.2, 0.2, 0.2);
	// Up (The new, smaller top face at Y = 0)
	glTexCoord2d(1, 0);
	glVertex3f(-1.25, 0, -1.5);
	glTexCoord2d(1, 1);
	glVertex3f(-1.25, 0, 0.0);
	glTexCoord2d(0, 1);
	glVertex3f(-0.25, 0, 0.0);
	glTexCoord2d(0, 0);
	glVertex3f(-0.25, 0, -1.5);

	// Left
	glTexCoord2d(1, 0);
	glVertex3f(-1.5, -1, 0.5);   // Base matches your old box
	glTexCoord2d(1, 1);
	glVertex3f(-1.5, -1, -2);
	glTexCoord2d(0, 1);
	glVertex3f(-1.25, 0, -1.5);  // Tapers to the new top
	glTexCoord2d(0, 0);
	glVertex3f(-1.25, 0, 0.0);

	// Back
	glTexCoord2d(1, 0);
	glVertex3f(-1.5, -1, -2);
	glTexCoord2d(1, 1);
	glVertex3f(0, -1, -2);
	glTexCoord2d(0, 1);
	glVertex3f(-0.25, 0, -1.5);
	glTexCoord2d(0, 0);
	glVertex3f(-1.25, 0, -1.5);

	// Right
	glTexCoord2d(1, 0);
	glVertex3f(0, -1, -2);
	glTexCoord2d(1, 1);
	glVertex3f(0, -1, 0.5);
	glTexCoord2d(0, 1);
	glVertex3f(-0.25, 0, 0.0);
	glTexCoord2d(0, 0);
	glVertex3f(-0.25, 0, -1.5);

	// Front 
	glTexCoord2d(1, 0);
	glVertex3f(0, -1, 0.5);
	glTexCoord2d(1, 1);
	glVertex3f(-1.5, -1, 0.5);
	glTexCoord2d(0, 1);
	glVertex3f(-1.25, 0, 0.0);
	glTexCoord2d(0, 0);
	glVertex3f(-0.25, 0, 0.0);

	// Down
	glTexCoord2d(1, 0);
	glVertex3f(-1.5, -1, -2);
	glTexCoord2d(1, 1);
	glVertex3f(-1.5, -1, 0.5);
	glTexCoord2d(0, 1);
	glVertex3f(0, -1, 0.5);
	glTexCoord2d(0, 0);
	glVertex3f(0, -1, -2);
	glEnd();
	glDeleteTextures(1, &clothTexture);

	glColor3f(0, 0, 0);
	clothTexture = loadTexture("brown+fabric-64x64.bmp");
	glBegin(GL_QUADS);
		// Up 
		glTexCoord2d(1, 0);
		glVertex3f(-1.5, -1, -2); 
		glTexCoord2d(1, 1);
		glVertex3f(-1.5, -1, 0.5); 
		glTexCoord2d(0, 1);
		glVertex3f(0, -1, 0.5); 
		glTexCoord2d(0, 0);
		glVertex3f(0, -1, -2);     

		// Left
		glTexCoord2d(1, 0);
		glVertex3f(-1.5, -1, 0.5); 
		glTexCoord2d(1, 1);
		glVertex3f(-1.5, -1, -2);  
		glTexCoord2d(0, 1);
		glVertex3f(-1.5, -2, -2);  
		glTexCoord2d(0, 0);
		glVertex3f(-1.5, -2, 0.5); 

		// Back
		glTexCoord2d(1, 0);
		glVertex3f(-1.5, -1, -2); 
		glTexCoord2d(1, 1);
		glVertex3f(0, -1, -2);     
		glTexCoord2d(0, 1);
		glVertex3f(-0.25, -2, -2); 
		glTexCoord2d(0, 0);
		glVertex3f(-1.5, -2, -2);  

		// Right
		glTexCoord2d(1, 0);
		glVertex3f(0, -1, -2);
		glTexCoord2d(1, 1);
		glVertex3f(0, -1, 0.5); 
		glTexCoord2d(0, 1);
		glVertex3f(-0.25, -2, 0.5);
		glTexCoord2d(0, 0);
		glVertex3f(-0.25, -2, -2); 

		// Front 
		glTexCoord2d(1, 0);
		glVertex3f(0, -1, 0.5);
		glTexCoord2d(1, 1);
		glVertex3f(-1.5, -1, 0.5); 
		glTexCoord2d(0, 1);
		glVertex3f(-1.5, -2, 0.5);
		glTexCoord2d(0, 0);
		glVertex3f(-0.25, -2, 0.5);

		// Down
		glTexCoord2d(1, 0);
		glVertex3f(-1.5, -2, -2);
		glTexCoord2d(1, 1);
		glVertex3f(-1.5, -2, 0.5);
		glTexCoord2d(0, 1);
		glVertex3f(-0.25, -2, 0.5);
		glTexCoord2d(0, 0);
		glVertex3f(-0.25, -2, -2); 
	glEnd();
	glDeleteTextures(1, &clothTexture);

	glColor3f(0.5, 0, 0);
	clothTexture = loadTexture("brown+fabric-64x64.bmp");
	glBegin(GL_QUADS);
		// Up (viewed from above, CCW)
		glTexCoord2d(1, 0);
		glVertex3f(-1.5, -2, -2);
		glTexCoord2d(1, 1);
		glVertex3f(-0.25, -2, -2);
		glTexCoord2d(0, 1);
		glVertex3f(-0.25, -2, 0.5);
		glTexCoord2d(0, 0);
		glVertex3f(-1.5, -2, 0.5);

		// Bottom (viewed from below, CCW)
		glTexCoord2d(1, 0);
		glVertex3f(-1.25, -3.75, -1.5);
		glTexCoord2d(1, 1);
		glVertex3f(-0.5, -3.75, -1.5);
		glTexCoord2d(0, 1);
		glVertex3f(-0.5, -3.75, 0);
		glTexCoord2d(0, 0);
		glVertex3f(-1.25, -3.75, 0);

		// Front (viewed from -Z, CCW)
		glTexCoord2d(1, 0);
		glVertex3f(-1.5, -2, -2);
		glTexCoord2d(1, 1);
		glVertex3f(-1.25, -3.75, -1.5);
		glTexCoord2d(0, 1);
		glVertex3f(-0.5, -3.75, -1.5);
		glTexCoord2d(0, 0);
		glVertex3f(-0.25, -2, -2);

		// Back (viewed from +Z, CCW)
		glTexCoord2d(1, 0);
		glVertex3f(-0.25, -2, 0.5);
		glTexCoord2d(1, 1);
		glVertex3f(-0.5, -3.75, 0);
		glTexCoord2d(0, 1);
		glVertex3f(-1.25, -3.75, 0);
		glTexCoord2d(0, 0);
		glVertex3f(-1.5, -2, 0.5);

		// Left (viewed from -X, CCW)
		glTexCoord2d(1, 0);
		glVertex3f(-1.5, -2, 0.5);
		glTexCoord2d(1, 1);
		glVertex3f(-1.5, -2, -2);
		glTexCoord2d(0, 1);
		glVertex3f(-1.25, -3.75, -1.5);
		glTexCoord2d(0, 0);
		glVertex3f(-1.25, -3.75, 0);

		// Right (viewed from +X, CCW)
		glTexCoord2d(1, 0);
		glVertex3f(-0.25, -2, -2);
		glTexCoord2d(1, 1);
		glVertex3f(-0.25, -2, 0.5);
		glTexCoord2d(0, 1);
		glVertex3f(-0.5, -3.75, 0);
		glTexCoord2d(0, 0);
		glVertex3f(-0.5, -3.75, -1.5);
	glEnd();
	glDeleteTextures(1, &clothTexture);
	
	//Joint
	glColor3f(0.2, 0.2, 0.2);
	glPushMatrix();
		glTranslatef(-0.9, -3.8, -0.75);
		glScalef(1, 1, 1.5);
		sphere(0.5);
	glPopMatrix();

	// Lower Leg
	clothTexture = loadTexture("brown+fabric-64x64.bmp");
	glColor3f(0.2, 0, 0);
	glPushMatrix();
	glBegin(GL_QUADS);
		// Up
		glTexCoord2d(1, 0);
		glVertex3f(-1.25, -3.9, -1.5);
		glTexCoord2d(1, 1);
		glVertex3f(-0.5, -3.9, -1.5);
		glTexCoord2d(0, 1);
		glVertex3f(-0.5, -3.9, 0);
		glTexCoord2d(0, 0);
		glVertex3f(-1.25, -3.9, 0);

		// Left
		glTexCoord2d(1, 0);
		glVertex3f(-1.25, -3.9, 0);
		glTexCoord2d(1, 1);
		glVertex3f(-1.25, -5, 0);
		glTexCoord2d(0, 1);
		glVertex3f(-1.25, -5, -1.5);
		glTexCoord2d(0, 0);
		glVertex3f(-1.25, -3.9, -1.5);

		// Back
		glTexCoord2d(1, 0);
		glVertex3f(-1.25, -3.9, -1.5);
		glTexCoord2d(1, 1);
		glVertex3f(-1.25, -5, -1.5);
		glTexCoord2d(0, 1);
		glVertex3f(-0.5, -5, -1.5);
		glTexCoord2d(0, 0);
		glVertex3f(-0.5, -3.9, -1.5);

		// Right
		glTexCoord2d(1, 0);
		glVertex3f(-0.5, -3.9, -1.5);
		glTexCoord2d(1, 1);
		glVertex3f(-0.5, -5, -1.5);
		glTexCoord2d(0, 1);
		glVertex3f(-0.5, -5, 0);
		glTexCoord2d(0, 0);
		glVertex3f(-0.5, -3.9, 0);

		// Front
		glTexCoord2d(1, 0);
		glVertex3f(-0.5, -3.9, 0);
		glTexCoord2d(1, 1);
		glVertex3f(-1.25, -3.9, 0);
		glTexCoord2d(0, 1);
		glVertex3f(-1.25, -5, 0);
		glTexCoord2d(0, 0);
		glVertex3f(-0.5, -5, 0);

		// Bottom
		glTexCoord2d(1, 0);
		glVertex3f(-0.5, -5, 0);
		glTexCoord2d(1, 1);
		glVertex3f(-1.25, -5, 0);
		glTexCoord2d(0, 1);
		glVertex3f(-1.25, -5, -1.5);
		glTexCoord2d(0, 0);
		glVertex3f(-0.5, -5, -1.5);
	glEnd();
	glDeleteTextures(1, &clothTexture);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.5, 0.5, 0.5);
	clothTexture = loadTexture("brown+fabric-64x64.bmp");
		glBegin(GL_QUADS);
		// Up
		glTexCoord2d(1, 0);
		glVertex3f(-1.25, -5, -1.5);
		glTexCoord2d(1, 1);
		glVertex3f(-0.5, -5, -1.5);
		glTexCoord2d(0, 1);
		glVertex3f(-0.5, -5, 0);
		glTexCoord2d(0, 0);
		glVertex3f(-1.25, -5, 0);

		// Left
		glTexCoord2d(1, 0);
		glVertex3f(-1.25, -5, 0);
		glTexCoord2d(1, 1);
		glVertex3f(-1.15, -7, 0);
		glTexCoord2d(0, 1);
		glVertex3f(-1.15, -7, -1.5);
		glTexCoord2d(0, 0);
		glVertex3f(-1.25, -5, -1.5);

		// Back
		glTexCoord2d(1, 0);
		glVertex3f(-1.25, -5, -1.5);
		glTexCoord2d(1, 1);
		glVertex3f(-1.15, -7, -1.5);
		glTexCoord2d(0, 1);
		glVertex3f(-0.65, -7, -1.5);
		glTexCoord2d(0, 0);
		glVertex3f(-0.5, -5, -1.5);

		// Right
		glTexCoord2d(1, 0);
		glVertex3f(-0.5, -5, -1.5);
		glTexCoord2d(1, 1);
		glVertex3f(-0.65, -7, -1.5);
		glTexCoord2d(0, 1);
		glVertex3f(-0.65, -7, 0);
		glTexCoord2d(0, 0);
		glVertex3f(-0.5, -5, 0);

		// Front
		glTexCoord2d(1, 0);
		glVertex3f(-0.5, -5, 0);
		glTexCoord2d(1, 1);
		glVertex3f(-1.25, -5, 0);
		glTexCoord2d(0, 1);
		glVertex3f(-1.15, -7, 0);
		glTexCoord2d(0, 0);
		glVertex3f(-0.65, -7, 0);

		// Bottom
		glTexCoord2d(1, 0);
		glVertex3f(-0.65, -7, 0);
		glTexCoord2d(1, 1);
		glVertex3f(-1.15, -7, 0);
		glTexCoord2d(0, 1);
		glVertex3f(-1.15, -7, -1.5);
		glTexCoord2d(0, 0);
		glVertex3f(-0.65, -7, -1.5);
		glEnd();
		glDeleteTextures(1, &clothTexture);
	glPopMatrix();

	glPopMatrix();
}

void lowerBody() {
	GLuint texture = 0;
	GLuint waistTexture = 0;
 	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	//Waist
	waistTexture = loadTexture("metallic+stripes-64x64.bmp");
	glColor3f(0.7, 0.7, 0.7);
	glBegin(GL_QUADS);
		//Upper Waist
		glTexCoord2f(0, 1);
		glVertex3f(-1.5, 0, -1.5);
		glTexCoord2f(1, 1);
		glVertex3f(1.5, 0, -1.5);
		glTexCoord2f(1, 0);
		glVertex3f(1.5, 0, 0);
		glTexCoord2f(0, 0);
		glVertex3f(-1.5, 0, 0);

		//Left face
		glTexCoord2f(0, 1);
		glVertex3f(-1.5, 0, 0.);
		glTexCoord2f(1, 1);
		glVertex3f(-1.5, -1, 0.5);
		glTexCoord2f(1, 0);
		glVertex3f(-1.5	, -1, -2);
		glTexCoord2f(0, 0);
		glVertex3f(-1.5, 0, -1.5);
		
		//Front Face
		glTexCoord2f(0, 1);
		glVertex3f(-1.5, 0, -1.5);
		glTexCoord2f(1, 1);
		glVertex3f(-1.5, -1, -2);
		glTexCoord2f(1, 0);
		glVertex3f(1.5, -1, -2);
		glTexCoord2f(0, 0);
		glVertex3f(1.5, 0, -1.5);

		//Right Face
		glTexCoord2f(0, 1);
		glVertex3f(1.5, 0, 0);
		glTexCoord2f(1, 1);
		glVertex3f(1.5, -1, 0.5);
		glTexCoord2f(1, 0);
		glVertex3f(1.5, -1, -2);
		glTexCoord2f(0, 0);
		glVertex3f(1.5, 0, -1.5);

		//Back Face
		glTexCoord2f(0, 0);
		
		glVertex3f(1.5, 0, 0);
		glTexCoord2f(0, 1);
		glVertex3f(-1.5, 0, 0);
		glTexCoord2f(1, 0);
		glVertex3f(-1.5, -1, 0.5);
		glTexCoord2f(1, 1);
		glVertex3f(1.5, -1, 0.5);
		
		//Lower Waist
		glTexCoord2f(0, 1);
		glVertex3f(1.5, -1, 0.5); // 4
		glTexCoord2f(1, 1);
		glVertex3f(-1.5, -1, 0.5); // 2
		glTexCoord2f(1, 0);
		glVertex3f(-1.5, -1, -2); // 1
		glTexCoord2f(0, 0);
		glVertex3f(1.5, -1, -2); // 3
	glEnd();
	glDeleteTextures(1, &waistTexture);

	glColor3f(0.1, 0.1, 0.1);
	glPushMatrix();
		glTranslatef(-1.5, -1.25, -0.75);
		glRotated(90, 0, 1, 0);
		cylinder(1.1, 1.1, 3);

	glPopMatrix();

	// Left Leg
	glColor3f(0.15, 0.15, 0.15);
	glPushMatrix();
	glTranslatef(-0.875, -0.5, -0.75); // Move pivot to leg's top-center attachment
	glRotatef(left_leg_angle, 1, 0, 0);          // Rotate around that pivot
	glTranslatef(0.875, 1, 0.75);    // Move back, but offset to correct world position
	glTranslatef(0, -1, 0);          // Final placement below waist
	leg();
	foot();
	glPopMatrix();

	// Right Leg
	glColor3f(0.15, 0.15, 0.15);
	//glColor3f(1, 0, 0);
	glPushMatrix();
	glTranslatef(0.875, -0.5, -0.75);  // Mirror pivot for right side
	glRotatef(right_leg_angle, 1, 0, 0);
	glTranslatef(-0.875, 1, 0.75);
	glTranslatef(0, -1, 0);
	glPushMatrix();
	glTranslatef(0, 0, -1.5);
	glRotatef(180, 0, 1, 0);
	leg();
	glPushMatrix();
	glTranslatef(-1.75, 0, -1.5);
	glRotatef(180, 0, 1, 0);
	foot();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	if (armorOn)
	{
		glPushMatrix();
		glRotatef(-10, 1, 0, 0);
		glTranslatef(-1.5, -1, 0);
		for (int i = 0; i < 7; i++)
		{
			for (int j = 0; j < 12; j++)
			{
				glPushMatrix();

				glTranslatef(i * 0.5f, j * -0.6, 0.75);
				glColor3f(1, 1, 0.5);
				texture = loadTexture("rusty+metal-32x32.bmp");
				disk(0.5, 0.7);
				glDeleteTextures(1, &texture);
				glPopMatrix();
			}
		}
		glPopMatrix();

		glPushMatrix();
		glRotatef(-90, 0, 1, 0);
		glRotatef(7, 1, 0, 0);
		glTranslatef(-1.5, -1, -2.3);
		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 12; j++)
			{
				glPushMatrix();

				glTranslatef(i * 0.5f, j * -0.6, 0.75);
				glColor3f(1, 1, 0.5);
				texture = loadTexture("rusty+metal-32x32.bmp");
				disk(0.5, 0.7);
				glDeleteTextures(1, &texture);
				glPopMatrix();
			}
		}
		glPopMatrix();

		glPushMatrix();
		glRotatef(90, 0, 1, 0);
		glRotatef(7, 1, 0, 0);
		glTranslatef(-0.5, -1, -2.3);
		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 12; j++)
			{
				glPushMatrix();

				glTranslatef(i * 0.5f, j * -0.6, 0.75);
				glColor3f(1, 1, 0.5);
				texture = loadTexture("rusty+metal-32x32.bmp");
				disk(0.5, 0.7);
				glDeleteTextures(1, &texture);
				glPopMatrix();
			}
		}
		glPopMatrix();
	}
}

// Perspective
void projection() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glRotatef(prx, 1, 0, 0);
	glRotatef(pry, 0, 1, 0);
	glRotatef(prz, 0, 0, 1);
	glTranslatef(ptx, pty, ptz);
	
	if (isOrtho) {
		glOrtho(
			oLeft * zoom,   // left
			oRight * zoom,   // right
			oBottom * zoom,  // bottom
			oTop * zoom,   // top
			-100,            // near - fixed, not zoomed
			100              // far  - fixed, not zoomed
		);
	}
	else {
		gluPerspective(45.0, 1.0, 1.0, 100.0);
	}
}

void display()
{
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	if (lightOn)
	{
		glEnable(GL_LIGHTING);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}

	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	GLfloat lightPos[] = { 3.0, 5.0, 5.0, 1.0 };
	GLfloat ambientLight[] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat diffuseLight[] = { 0.8, 0.8, 0.8, 1.0};

	character_facing_angle += (target_facing_angle - character_facing_angle) * 0.05;

	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);

	projection();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (isOrtho)
	{
		glTranslatef(0.0, -0.5, 0.0);
	}
	else {
		glTranslatef(0.0, -0.5, -4.5 + tz);
	}
	
	glRotatef(objRot, 0.0, 1.0, 0.0);

	attacking();

	glPushMatrix();
	glScalef(3, 3, 3);
	glTranslatef(0, -1, -1.5);
	background();
	glPopMatrix();


	//Character
	//glTranslatef(-2.5, -2.5, 0);
	glPushMatrix();
	glScalef(0.5, 0.5, 0.5);

	//Character Facing Direction
	glPushMatrix();
	glTranslatef(mx, my, mz);

	//Character Movement
	glPushMatrix();
	glRotatef(character_facing_angle, 0, 1, 0);

	// Upper Body
	glPushMatrix();
		glScalef(4, 4, 4);
		character();
	glPopMatrix();

	// Lower Body, (Scale based on upper)
	glPushMatrix();
		glRotatef(180, 0, 1, 0);
		glTranslatef(0, -1.5, 0.65);
		glScalef(1.1, 1.0, 0.82);
		lowerBody();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}
//--------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
	initRain();
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = WindowProcedure;
	wc.lpszClassName = WINDOW_TITLE;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc)) return false;

	HWND hWnd = CreateWindow(WINDOW_TITLE, WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 800,
		NULL, NULL, wc.hInstance, NULL);

	//--------------------------------
	//	Initialize window for OpenGL
	//--------------------------------

	HDC hdc = GetDC(hWnd);

	//	initialize pixel format for the window
	initPixelFormat(hdc);

	//	get an openGL context
	HGLRC hglrc = wglCreateContext(hdc);

	//	make context current
	if (!wglMakeCurrent(hdc, hglrc)) return false;

	//--------------------------------
	//	End initialization
	//--------------------------------

	roofTex = loadTexture("roof.bmp");
    redWallTex = loadTexture("redWall.bmp");
    greyWallTex = loadTexture("greyWall.bmp");
    windowTex = loadTexture("window.bmp");
    doorTex = loadTexture("door.bmp");
    houseWallTex = loadTexture("houseWall.bmp");
    floorTex = loadTexture("floor.bmp");
    lanternTex = loadTexture("lantern.bmp");
    pathWayTex = loadTexture("pathWay.bmp");
    tableTex = loadTexture("table.bmp");


	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		display();
		drawRain();
		SwapBuffers(hdc);
	}

	UnregisterClass(WINDOW_TITLE, wc.hInstance);

	return true;
}
//--------------------------------------------------------------------