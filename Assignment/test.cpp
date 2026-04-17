#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#define WINDOW_TITLE "OpenGL Window"

BITMAP BMP;
HBITMAP hBMP = NULL;

LRESULT WINAPI WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) PostQuitMessage(0);
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

void drawCube(float sz) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_QUADS);
	//glColor3f(1, 0, 0);
	glTexCoord2f(0, 1);
	glVertex3f(0, 0, sz);
	glTexCoord2f(1, 1);
	glVertex3f(sz, 0, sz);
	glTexCoord2f(1, 0);
	glVertex3f(sz, 0, 0);
	glTexCoord2f(0, 0);
	glVertex3f(0, 0, 0);

	//glColor3f(1, 1, 0);
	glTexCoord2f(0, 0);
	glVertex3f(0, 0, 0);
	glTexCoord2f(1, 0);
	glVertex3f(0, sz, 0);
	glTexCoord2f(1, 1);
	glVertex3f(0, sz, sz);
	glTexCoord2f(0, 1);
	glVertex3f(0, 0, sz);

	//glColor3f(0, 1, 0);
	glTexCoord2f(0, 1);
	glVertex3f(0, 0, sz);
	glTexCoord2f(1, 1);
	glVertex3f(0, sz, sz);
	glTexCoord2f(1, 0);
	glVertex3f(sz, sz, sz);
	glTexCoord2f(0, 0);
	glVertex3f(sz, 0, sz);

	//glColor3f(0, 1, 1);
	glTexCoord2f(0, 0);
	glVertex3f(sz, 0, sz);
	glTexCoord2f(1, 0);
	glVertex3f(sz, sz, sz);
	glTexCoord2f(1, 1);
	glVertex3f(sz, sz, 0);
	glTexCoord2f(0, 1);
	glVertex3f(sz, 0, 0);

	//glColor3f(0, 0, 1);
	glTexCoord2f(0, 1);
	glVertex3f(sz, 0, 0);
	glTexCoord2f(1, 1);
	glVertex3f(0, 0, 0);
	glTexCoord2f(1, 0);
	glVertex3f(0, sz, 0);
	glTexCoord2f(0, 0);
	glVertex3f(sz, sz, 0);

	//glColor3f(1, 0, 1);
	glTexCoord2f(0, 0);
	glVertex3f(sz, sz, 0);
	glTexCoord2f(1, 0);
	glVertex3f(0, sz, 0);
	glTexCoord2f(1, 1);
	glVertex3f(0, sz, sz);
	glTexCoord2f(0, 1);
	glVertex3f(sz, sz, sz);
	glEnd();
}

GLuint loadTexture(LPCSTR filepath) {
	// Take from step 1
	GLuint texture = 0;

	// Initialize texture info
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	HBITMAP hBMP = (HBITMAP)LoadImage(GetModuleHandle(NULL),
		filepath, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION |
		LR_LOADFROMFILE);
	GetObject(hBMP, sizeof(BMP), &BMP);

	glEnable(GL_TEXTURE_2D);
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

void sphere(double radius) {
	GLUquadricObj* sphere = NULL;
	sphere = gluNewQuadric();

	gluQuadricTexture(sphere, GL_FILL);
	gluQuadricTexture(sphere, GL_TRUE);
	gluSphere(sphere, radius, 50, 50);

	gluDeleteQuadric(sphere);
}


void display()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glRotatef(0.1, 1, 1, 1);

	GLuint textureArr[2];
	textureArr[0] = loadTexture("red+hot+fire+flames-64x64.bmp");
	sphere(0.5);
	glDeleteTextures(1, &textureArr[0]);

	glDisable(GL_TEXTURE_2D);
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