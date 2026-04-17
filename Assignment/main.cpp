#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>

#define WINDOW_TITLE "Assignment Character"

float oLeft = -10, oRight = 10, oBottom = -10, oTop = 10, oNear = -10, oFar = 10;
float ptx = 0, pty = 0, ptz = 0, pspeed = 0.1;
float p_rotate_angle = 0, p_angle_speed = 10;
float zoom = 1.0f, zoom_speed = 0.1f;
float left_leg_angle = 0, right_leg_angle = 0, maximum_leg_angle = 25, minimum_leg_angle = -25, leg_angle_speed = 2, leg_direction = 1;
int character_facing_angle = 0, character_rotation_speed = 90, target_facing_angle= 0;
float mx = 0, my = 0, mz = 0, mspeed = 0.1; //movement

//Light
bool lightOn = TRUE;
bool textureOn = TRUE;

//perspective
float tz = 0.0;
float tSpeed = 0.5;
float prx = 0, pry = 0, prz = 0, prspeed = 10;
bool isOrtho = true;

//texture mapping
BITMAP BMP;
HBITMAP hBMP = NULL;
GLuint roofTex;


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
		case VK_UP:
			pty += pspeed;
			break;
		case VK_DOWN:
			pty -= pspeed;
			break;
		case VK_LEFT:
			ptx -= pspeed;
			break;
		case VK_RIGHT:
			ptx += pspeed;
			break;

		case '+':
		case VK_OEM_PLUS:
			if (zoom < 5.0f)
			{
				zoom += zoom_speed;
			}
			tz += tSpeed; // zoom in
			break;

		case '-':
		case VK_OEM_MINUS:
			if (zoom > 0.2f)
			{
				zoom -= zoom_speed;
			}
			tz -= tSpeed; // zoom out
			break;

		case 'N':
			objRot -= 5.0; // spin left
			break;
		case 'M':
			objRot += 5.0; // spin right
			break;

		case 'O': isOrtho = true; tz = 0; break; // reset zoom if changing projection
		case 'P': isOrtho = false; tz = 0; break;

			//character movement
		case 'S':
			if (mz > oNear + 1)
			{
				mz -= mspeed;
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
		case 'W':
			if (mz < oFar - 1)
			{
				mz += mspeed;
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
		case '8':
			prx += prspeed;
			break;
		case '9':
			pry += prspeed;
			break;
		case '0':
			prz += prspeed;
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
	glVertex3f(0.0f, 1.0f, 0.0f);   // Apex
	glVertex3f(-0.5f, 0.0f, 0.5f);   // Bottom-left
	glVertex3f(0.5f, 0.0f, 0.5f);   // Bottom-right

	// Back face
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.5f, 0.0f, -0.5f);
	glVertex3f(-0.5f, 0.0f, -0.5f);

	// Left face
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.5f, 0.0f, -0.5f);
	glVertex3f(-0.5f, 0.0f, 0.5f);

	// Right face
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.5f, 0.0f, 0.5f);
	glVertex3f(0.5f, 0.0f, -0.5f);

	glEnd();

	// Square base using GL_QUADS
	glBegin(GL_QUADS);
	glVertex3f(-0.5f, 0.0f, 0.5f);
	glVertex3f(0.5f, 0.0f, 0.5f);
	glVertex3f(0.5f, 0.0f, -0.5f);
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

void weapon() {
	//Spear Blade
	glPushMatrix();
	glTranslatef(0, 4, 0.1);

	//Lower Blade
	glColor3f(1, 1, 1);
	glPushMatrix();
	glRotatef(180, 1, 0, 0);
	glScalef(1.5, 1.5, 0.2);
	pyramid();
	glPopMatrix();

	//Upper Blade
	glColor3f(1, 1, 1);
	glPushMatrix();
	glScalef(1.5, 3.25, 0.2);
	pyramid();
	glPopMatrix();
	glPopMatrix();

	//Ring blade between
	glColor3f(1, 1, 1);
	glPushMatrix();
	glScalef(0.7, 0.7, 0.7);
	glTranslatef(0, 2, 0);
	glPushMatrix();
	glTranslatef(0, 0, 0.1);
	disk(1.35, 1.45);
	glPopMatrix();
	cylinder(1.45, 1.45, 0.2);
	glPushMatrix();
	glTranslatef(0, 0, 0.1);
	disk(1.45, 1.65);
	glPopMatrix();
	glPopMatrix();

	//Handle
	glColor3f(1, 1, 1);
	glPushMatrix();
	glTranslatef(0, 3, 0.1);
	glPushMatrix();
	glRotatef(-90, 0, 0, 1);
	glRotatef(90, 0, 1, 0);
	cylinder(0.15, 0.15, 12);
	glPopMatrix();
	glPopMatrix();

	glColor3f(0.65, 0.35, 0);
	glPushMatrix();
	glTranslatef(0, 3, 0.1);
	glRotatef(-90, 0, 0, 1);
	glRotatef(90, 0, 1, 0);
	cylinder(0.2, 0.2, 3);
	glPopMatrix();

	//Lower Handle Cover
	glColor3f(1, 1, 1);
	glPushMatrix();
	glTranslatef(0, -9, 0.1);
	sphere(0.2);
	glPushMatrix();
	glRotatef(-90, 0, 0, 1);
	glRotatef(90, 0, 1, 0);
	cone(0.15, 1);
	glPopMatrix();
	glPopMatrix();

	//Upper Handle Cover
	glPushMatrix();
	glTranslatef(0, 3, 0.1);
	sphere(0.35);

	// Left Blade
	glPushMatrix();
	glRotatef(90, 0, 1, 0);
	cylinder(0.15, 0.15, 1);
	glPushMatrix();
	glTranslatef(0, 0, 1);
	sphere(0.15);

	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	glTranslatef(0, -0.4, 0);
	glScalef(0.1, 0.8, 0.5);
	pyramid();
	glPopMatrix();

	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glTranslatef(0, 0.4, 0);
	glScalef(0.1, 1.25, 0.5);
	pyramid();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	//Right Blade
	glPushMatrix();
	glRotatef(-90, 0, 1, 0);
	cylinder(0.15, 0.15, 1);
	glPushMatrix();
	glTranslatef(0, 0, 1);
	sphere(0.15);

	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	glTranslatef(0, -0.4, 0);
	glScalef(0.1, 0.8, 0.5);
	pyramid();
	glPopMatrix();

	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glTranslatef(0, 0.4, 0);
	glScalef(0.1, 1.25, 0.5);
	pyramid();
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
			if(handIndex == 1)
				weapon();
	
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
	// Left Leg
	glPushMatrix();

	glBegin(GL_QUADS);
	// Up (The new, smaller top face at Y = 0)
	glVertex3f(-1.25, 0, -1.5);
	glVertex3f(-1.25, 0, 0.0);
	glVertex3f(-0.25, 0, 0.0);
	glVertex3f(-0.25, 0, -1.5);

	// Left
	glVertex3f(-1.5, -1, 0.5);   // Base matches your old box
	glVertex3f(-1.5, -1, -2);
	glVertex3f(-1.25, 0, -1.5);  // Tapers to the new top
	glVertex3f(-1.25, 0, 0.0);

	// Back
	glVertex3f(-1.5, -1, -2);
	glVertex3f(0, -1, -2);
	glVertex3f(-0.25, 0, -1.5);
	glVertex3f(-1.25, 0, -1.5);

	// Right 
	glVertex3f(0, -1, -2);
	glVertex3f(0, -1, 0.5);
	glVertex3f(-0.25, 0, 0.0);
	glVertex3f(-0.25, 0, -1.5);

	// Front 
	glVertex3f(0, -1, 0.5);
	glVertex3f(-1.5, -1, 0.5);
	glVertex3f(-1.25, 0, 0.0);
	glVertex3f(-0.25, 0, 0.0);

	// Down (The exact same coordinates as your old "Up" face)
	glVertex3f(-1.5, -1, -2);
	glVertex3f(-1.5, -1, 0.5);
	glVertex3f(0, -1, 0.5);
	glVertex3f(0, -1, -2);
	glEnd();

	glBegin(GL_QUADS);
		// Up 
		glVertex3f(-1.5, -1, -2);  
		glVertex3f(-1.5, -1, 0.5); 
		glVertex3f(0, -1, 0.5);    
		glVertex3f(0, -1, -2);     

		// Left
		glVertex3f(-1.5, -1, 0.5); 
		glVertex3f(-1.5, -1, -2);  
		glVertex3f(-1.5, -2, -2);  
		glVertex3f(-1.5, -2, 0.5); 

		// Back
		glVertex3f(-1.5, -1, -2); 
		glVertex3f(0, -1, -2);     
		glVertex3f(-0.25, -2, -2); 
		glVertex3f(-1.5, -2, -2);  

		// Right 
		glVertex3f(0, -1, -2);     
		glVertex3f(0, -1, 0.5);   
		glVertex3f(-0.25, -2, 0.5);
		glVertex3f(-0.25, -2, -2); 

		// Front 
		glVertex3f(0, -1, 0.5);   
		glVertex3f(-1.5, -1, 0.5); 
		glVertex3f(-1.5, -2, 0.5);
		glVertex3f(-0.25, -2, 0.5);

		// Down
		glVertex3f(-1.5, -2, -2); 
		glVertex3f(-1.5, -2, 0.5);
		glVertex3f(-0.25, -2, 0.5);
		glVertex3f(-0.25, -2, -2); 
	glEnd();

	glBegin(GL_QUADS);
		// Up (viewed from above, CCW)
		glVertex3f(-1.5, -2, -2);
		glVertex3f(-0.25, -2, -2);
		glVertex3f(-0.25, -2, 0.5);
		glVertex3f(-1.5, -2, 0.5);

		// Bottom (viewed from below, CCW)
		glVertex3f(-1.25, -3.75, -1.5);
		glVertex3f(-0.5, -3.75, -1.5);
		glVertex3f(-0.5, -3.75, 0);
		glVertex3f(-1.25, -3.75, 0);

		// Front (viewed from -Z, CCW)
		glVertex3f(-1.5, -2, -2);
		glVertex3f(-1.25, -3.75, -1.5);
		glVertex3f(-0.5, -3.75, -1.5);
		glVertex3f(-0.25, -2, -2);

		// Back (viewed from +Z, CCW)
		glVertex3f(-0.25, -2, 0.5);
		glVertex3f(-0.5, -3.75, 0);
		glVertex3f(-1.25, -3.75, 0);
		glVertex3f(-1.5, -2, 0.5);

		// Left (viewed from -X, CCW)
		glVertex3f(-1.5, -2, 0.5);
		glVertex3f(-1.5, -2, -2);
		glVertex3f(-1.25, -3.75, -1.5);
		glVertex3f(-1.25, -3.75, 0);

		// Right (viewed from +X, CCW)
		glVertex3f(-0.25, -2, -2);
		glVertex3f(-0.25, -2, 0.5);
		glVertex3f(-0.5, -3.75, 0);
		glVertex3f(-0.5, -3.75, -1.5);
	glEnd();
	
	//Joint
	glPushMatrix();
		glTranslatef(-0.9, -3.8, -0.75);
		glScalef(1, 1, 1.5);
		sphere(0.5);
	glPopMatrix();

	// Lower Leg
	glPushMatrix();
	glBegin(GL_QUADS);
		// Up
		glVertex3f(-1.25, -3.9, -1.5);
		glVertex3f(-0.5, -3.9, -1.5);
		glVertex3f(-0.5, -3.9, 0);
		glVertex3f(-1.25, -3.9, 0);

		// Left
		glVertex3f(-1.25, -3.9, 0);
		glVertex3f(-1.25, -5, 0);
		glVertex3f(-1.25, -5, -1.5);
		glVertex3f(-1.25, -3.9, -1.5);

		// Back
		glVertex3f(-1.25, -3.9, -1.5);
		glVertex3f(-1.25, -5, -1.5);
		glVertex3f(-0.5, -5, -1.5);
		glVertex3f(-0.5, -3.9, -1.5);

		// Right
		glVertex3f(-0.5, -3.9, -1.5);
		glVertex3f(-0.5, -5, -1.5);
		glVertex3f(-0.5, -5, 0);
		glVertex3f(-0.5, -3.9, 0);

		// Front
		glVertex3f(-0.5, -3.9, 0);
		glVertex3f(-1.25, -3.9, 0);
		glVertex3f(-1.25, -5, 0);
		glVertex3f(-0.5, -5, 0);

		// Bottom
		glVertex3f(-0.5, -5, 0);
		glVertex3f(-1.25, -5, 0);
		glVertex3f(-1.25, -5, -1.5);
		glVertex3f(-0.5, -5, -1.5);
	glEnd();
	glPopMatrix();


	glPushMatrix();
	glColor3f(0.95, 0.8, 0.7);
		glBegin(GL_QUADS);
		// Up
		glVertex3f(-1.25, -5, -1.5);
		glVertex3f(-0.5, -5, -1.5);
		glVertex3f(-0.5, -5, 0);
		glVertex3f(-1.25, -5, 0);

		// Left
		glVertex3f(-1.25, -5, 0);
		glVertex3f(-0.65, -7, 0);
		glVertex3f(-0.65, -7, -1.5);
		glVertex3f(-1.25, -5, -1.5);

		// Back
		glVertex3f(-1.25, -5, -1.5);
		glVertex3f(-1.15, -7, -1.5);
		glVertex3f(-0.65, -7, -1.5);
		glVertex3f(-0.5, -5, -1.5);

		// Right
		glVertex3f(-0.5, -5, -1.5);
		glVertex3f(-0.65, -7, -1.5);
		glVertex3f(-0.65, -7, 0);
		glVertex3f(-0.5, -5, 0);

		// Front
		glVertex3f(-0.5, -5, 0);
		glVertex3f(-1.25, -5, 0);
		glVertex3f(-1.15, -7, 0);
		glVertex3f(-0.65, -7, 0);

		// Bottom
		glVertex3f(-0.65, -7, 0);
		glVertex3f(-1.15, -7, 0);
		glVertex3f(-1.15, -7, -1.5);
		glVertex3f(-0.65, -7, -1.5);
		glEnd();
	glPopMatrix();

	glPopMatrix();
}

void lowerBody() {
 	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	//Wrist
	glColor3f(0.15, 0.15, 0.15);
	glBegin(GL_QUADS);
		//Upper wrist
		glVertex3f(-1.5, 0, -1.5);  
		glVertex3f(1.5, 0, -1.5);
		glVertex3f(1.5, 0, 0);
		glVertex3f(-1.5, 0, 0);

		//Left face
		glVertex3f(-1.5, 0, 0.5);
		glVertex3f(-1.5, -1, 0.5);
		glVertex3f(-1.5	, -1, -2);
		glVertex3f(-1.5, 0, -2);
		
		//Front Face
		glVertex3f(-1.5, 0, -1.5);
		glVertex3f(-1.5, -1, -2);
		glVertex3f(1.5, -1, -2);
		glVertex3f(1.5, 0, -1.5);

		//Right Face
		glVertex3f(1.5, 0, 0.5);
		glVertex3f(1.5, -1, 0.5);
		glVertex3f(1.5, -1, -2);
		glVertex3f(1.5, 0, -2);

		//Back Face
		glVertex3f(1.5, 0, 0);
		glVertex3f(-1.5, 0, 0);
		glVertex3f(-1.5, -1, 0.5);
		glVertex3f(1.5, -1, 0.5);
		
		//Lower Wrist
		glVertex3f(1.5, -1, 0.5); // 4
		glVertex3f(-1.5, -1, 0.5); // 2
		glVertex3f(-1.5, -1, -2); // 1
		glVertex3f(1.5, -1, -2); // 3
	glEnd();

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
	glTranslatef(0, -1, 0);          // Final placement below wrist
	leg();
	foot();
	glPopMatrix();

	// Right Leg
	//glColor3f(0.15, 0.15, 0.15);
	glColor3f(1, 0, 0);
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
}


// Perspective
void projection() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glTranslatef(ptx, pty, ptz);
	glRotatef(prx, 1, 0, 0);
	glRotatef(pry, 0, 1, 0);
	glRotatef(prz, 0, 0, 1);

	if (isOrtho) {
		glOrtho(oLeft * zoom, oRight * zoom, oBottom * zoom, oTop * zoom, oNear * zoom, oFar * zoom);
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

	glTranslatef(0.0, -0.5, -4.5 + tz);
	glRotatef(objRot, 0.0, 1.0, 0.0);



	//Character
	//glTranslatef(-2.5, -2.5, 0);
	glPushMatrix();
	glTranslatef(0, 1, 0);

	//Character Facing Direction
	glPushMatrix();
	glRotatef(character_facing_angle, 0, 1, 0);

	//Character Movement
	glPushMatrix();
	glTranslatef(mx, my, mz);

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

		SwapBuffers(hdc);
	}

	UnregisterClass(WINDOW_TITLE, wc.hInstance);

	return true;
}
//--------------------------------------------------------------------