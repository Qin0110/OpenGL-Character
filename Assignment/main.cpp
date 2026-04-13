#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#define WINDOW_TITLE "Assignment Character"

float oLeft = -10, oRight = 10, oBottom = -10, oTop = 10, oNear = -10, oFar = 10;
float ptx = 0, pty = 0, ptz = 0, pspeed = 0.1;
float p_rotate_angle = 0, p_angle_speed = 10;
float zoom = 1.0f, zoom_speed = 0.1f;

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

//For testing
void cube(float sz) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, sz);
	glVertex3f(sz, 0, sz);
	glVertex3f(sz, 0, 0);
	glVertex3f(0, 0, 0);

	glColor3f(1, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, sz, 0);
	glVertex3f(0, sz, sz);
	glVertex3f(0, 0, sz);

	glColor3f(0, 1, 0);
	glVertex3f(0, 0, sz);
	glVertex3f(0, sz, sz);
	glVertex3f(sz, sz, sz);
	glVertex3f(sz, 0, sz);

	glColor3f(0, 1, 1);
	glVertex3f(sz, 0, sz);
	glVertex3f(sz, sz, sz);
	glVertex3f(sz, sz, 0);
	glVertex3f(sz, 0, 0);

	glColor3f(0, 0, 1);
	glVertex3f(sz, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, sz, 0);
	glVertex3f(sz, sz, 0);

	glColor3f(1, 0, 1);
	glVertex3f(sz, sz, 0);
	glVertex3f(0, sz, 0);
	glVertex3f(0, sz, sz);
	glVertex3f(sz, sz, sz);
	glEnd();
}


void leg() {

}


void lowerBody() {
	//Delete this line, debug usage
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	//Wrist
	glBegin(GL_QUADS);
		//Upper wrist
		glVertex3f(-2, 0, -1);  
		glVertex3f(2, 0, -1);   
		glVertex3f(2, 0, 0);  
		glVertex3f(-2, 0, 0);

		//Left face
		glVertex3f(-2, 0, 0);
		glVertex3f(-2, -1, 0.5);
		glVertex3f(-2, -1, -1.5);
		glVertex3f(-2, 0, -1);
		
		//Front Face
		glVertex3f(-2, 0, -1);
		glVertex3f(-2, -1, -1.5);
		glVertex3f(2, -1, -1.5);
		glVertex3f(2, 0, -1);

		//Right Face
		glVertex3f(2, 0, -1);
		glVertex3f(2, -1, -1.5);
		glVertex3f(2, -1, 0.5);
		glVertex3f(2, 0, 0);

		//Back Face
		glVertex3f(2, 0, 0);
		glVertex3f(-2, 0, 0);
		glVertex3f(-2, -1, 0.5);
		glVertex3f(2, -1, 0.5);
		
		//Lower Wrist
		glVertex3f(2, -1, 0.5); // 4
		glVertex3f(-2, -1, 0.5); // 2
		glVertex3f(-2, -1, -1.5); // 1
		glVertex3f(2, -1, -1.5); // 3
	glEnd();
}



// Perspective
void projection() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glTranslatef(ptx, pty, ptz);
	glRotatef(45, 0, 0, 1);

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
	glRotatef(0.01, 1, 1, 1);
	leg();
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