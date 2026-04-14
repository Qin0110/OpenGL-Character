#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#define WINDOW_TITLE "Assignment Character"

float oLeft = -10, oRight = 10, oBottom = -10, oTop = 10, oNear = -10, oFar = 10;
float ptx = 0, pty = 0, ptz = 0, pspeed = 0.1;
float p_rotate_angle = 0, p_angle_speed = 10;
float zoom = 1.0f, zoom_speed = 0.1f;
float left_leg_angle = 0, right_leg_angle = 0, maximum_leg_angle = 25, minimum_leg_angle = -25, leg_angle_speed = 2, leg_direction = 1;
int character_facing_angle = 0, character_rotation_speed = 90;
float mx = 0, my = 0, mz = 0, mspeed = 0.1; //movement

LRESULT WINAPI WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) PostQuitMessage(0);
		else if (wParam == VK_UP) {
			pty += pspeed;
		}
		else if (wParam == VK_DOWN) {
			pty -= pspeed;
		}
		else if (wParam == VK_LEFT) {
			ptx -= pspeed;
		}
		else if (wParam == VK_RIGHT) {
			ptx += pspeed;
		}
		else if (wParam == 'I') {
			if (zoom > 0.2f)
			{
				zoom -= zoom_speed;
			}
		}
		else if (wParam == 'O') {
			if (zoom < 5.0f)
			{
				zoom += zoom_speed;
			}
		}
		//character movement
		else if (wParam == 'W') {
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
		}
		else if (wParam == 'S') {
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
		}
		// character face directon
		else if (wParam == 'A') {
			character_facing_angle += character_rotation_speed;
			character_facing_angle == 360 ? character_facing_angle = 0 : character_facing_angle;
		}
		else if (wParam == 'D') {
			character_facing_angle -= character_rotation_speed;
			character_facing_angle == -360 ? character_facing_angle = 0 : character_facing_angle;
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

void cylinder(double base_Radius, double top_Radius, double height) {
	GLUquadricObj* cylinder = NULL;
	cylinder = gluNewQuadric();

	gluQuadricDrawStyle(cylinder, GLU_FILL);
	gluCylinder(cylinder, base_Radius, top_Radius, height, 50, 50);

	gluDeleteQuadric(cylinder);
}

void sphere(double radius) {
	GLUquadricObj* sphere = NULL;
	sphere = gluNewQuadric();

	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluSphere(sphere, radius, 50, 50);

	gluDeleteQuadric(sphere);
}

void foot() {
	glBegin(GL_QUADS);
		// Up
		glVertex3f(-0.5, -6, 0);
		glVertex3f(-1.25, -6, 0);
		glVertex3f(-1.25, -6, -1.5);
		glVertex3f(-0.5, -6, -1.5);

		// Front
		glVertex3f(-0.5, -6, -1.5);
		glVertex3f(-1.25, -6, -1.5);
		glVertex3f(-1.25, -6.75, -2.5);
		glVertex3f(-0.5, -6.75, -2.5);

		// Right
		glVertex3f(-0.5, -6.75, -2.5);
		glVertex3f(-0.5, -6.75, 0);
		glVertex3f(-0.5, -6, 0);
		glVertex3f(-0.5, -6, -1.5);

		// Bottom
		glVertex3f(-0.5, -6.75, 0);
		glVertex3f(-1.25, -6.75, 0);
		glVertex3f(-1.25, -6.75, -2.5);
		glVertex3f(-0.5, -6.75, -2.5);

		// Left
		glVertex3f(-1.25, -6.75, 0);
		glVertex3f(-1.25, -6, 0);
		glVertex3f(-1.25, -6, -1.5);
		glVertex3f(-1.25, -6.75, -2.5);

		// Back
		glVertex3f(-1.25, -6, -1.5);
		glVertex3f(-0.5, -6, -1.5);
		glVertex3f(-0.5, -6.75, -2.5);
		glVertex3f(-1.25, -6.75, -2.5);
	glEnd();
}

void leg() {
	// Left Leg
	glPushMatrix();
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
		glVertex3f(-1.25, -3.75, -1.5);
		glVertex3f(-0.5, -3.75, -1.5);
		glVertex3f(-0.5, -3.75, 0);
		glVertex3f(-1.25, -3.75, 0);

		// Left
		glVertex3f(-1.25, -3.9, 0);
		glVertex3f(-1.25, -6, 0);
		glVertex3f(-1.25, -6, -1.5);
		glVertex3f(-1.25, -3.9, -1.5);

		// Back
		glVertex3f(-1.25, -3.9, -1.5);
		glVertex3f(-1.25, -6, -1.5);
		glVertex3f(-0.5, -6, -1.5);
		glVertex3f(-0.5, -3.9, -1.5);

		// Right
		glVertex3f(-0.5, -3.9, -1.5);
		glVertex3f(-0.5, -6, -1.5);
		glVertex3f(-0.5, -6, 0);
		glVertex3f(-0.5, -3.9, 0);

		// Front
		glVertex3f(-0.5, -3.9, 0);
		glVertex3f(-1.25, -3.9, 0);
		glVertex3f(-1.25, -6, 0);
		glVertex3f(-0.5, -6, 0);

		// Bottom
		glVertex3f(-0.5, -6, 0);
		glVertex3f(-1.25, -6, 0);
		glVertex3f(-1.25, -6, -1.5);
		glVertex3f(-0.5, -6, -1.5);

	glEnd();
	glPopMatrix();

	glPopMatrix();
}

void lowerBody() {
	//Delete this line, debug usage
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	//Wrist
	glBegin(GL_QUADS);
		//Upper wrist
		glVertex3f(-1.5, 0, -1.5);  
		glVertex3f(1.5, 0, -1.5);
		glVertex3f(1.5, 0, 0);
		glVertex3f(-1.5, 0, 0);

		//Left face
		glVertex3f(-1.5, 0, 0);
		glVertex3f(-1.5, -1, 0.5);
		glVertex3f(-1.5	, -1, -2);
		glVertex3f(-1.5, 0, -1.5);
		
		//Front Face
		glVertex3f(-1.5, 0, -1.5);
		glVertex3f(-1.5, -1, -2);
		glVertex3f(1.5, -1, -2);
		glVertex3f(1.5, 0, -1.5);

		//Right Face
		glVertex3f(1.5, 0, -1.5);
		glVertex3f(1.5, -1, -2);
		glVertex3f(1.5, -1, 0.5);
		glVertex3f(1.5, 0, 0);

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

	glPushMatrix();
	glTranslatef(-1.5, -1.25, -0.75);
	glRotated(90, 0, 1, 0);
	cylinder(1.1, 1.1, 3);
	glPopMatrix();

	// Left Leg
	glPushMatrix();
	glTranslatef(-0.875, -0.5, -0.75); // Move pivot to leg's top-center attachment
	glRotatef(left_leg_angle, 1, 0, 0);          // Rotate around that pivot
	glTranslatef(0.875, 1, 0.75);    // Move back, but offset to correct world position
	glTranslatef(0, -1, 0);          // Final placement below wrist
	leg();
	foot();
	glPopMatrix();

	// Right Leg
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
	//glRotatef(45, 1, 1, 1);

	glOrtho(oLeft * zoom, oRight * zoom, oBottom * zoom, oTop * zoom, oNear * zoom, oFar * zoom);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 0);

	projection();

	glMatrixMode(GL_MODELVIEW);


	//glTranslatef(-2.5, -2.5, 0);
	glLoadIdentity();
	glRotatef(90, 0, 1, 0);
	
	//Character Facing Direction
	glPushMatrix();
	glRotatef(character_facing_angle, 0, 1, 0);

	//Character Movement
	glPushMatrix();
	glTranslatef(mx, my, mz);
	
	// Upper Body

	// Lower Body, (Scale based on upper)
	glPushMatrix();
	glScalef(1.35, 1.0, 0.8);
	lowerBody();
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