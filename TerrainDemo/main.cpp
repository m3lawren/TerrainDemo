#include <Terrain/Terrain.h>

// include the basic windows header files and the Direct3D header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>

#include <cmath>
#include <cstdio>
#include <ctime>

#undef M_PI
#define M_PI 3.141592653589793238462643f

// define the screen resolution
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define T_WIDTH		32.0f
#define T_HEIGHT	32.0f
#define T_STEPS		256

#define T_OFF(x,y) ((y) * ((T_STEPS) + 1) + (x))
#define T_Y(i) (-(T_HEIGHT / 2.0f) + (i) * T_HEIGHT / T_STEPS)
#define T_X(i) (-(T_WIDTH / 2.0f) + (i) * T_WIDTH / T_STEPS)

#define ANGLE_DELTA (float)(M_PI / 32.0f)

#define T_GENFUNC(x,y) (sin(2.0f * (x)+(y)) + sin((x)-1.5f * (y))) / 2.5f

#define CHECK_PITCH(p) if (p <= -M_PI / 2.0f) p = -M_PI / 2.0f + ANGLE_DELTA; else if (p >= M_PI / 2.0f) p = M_PI / 2.0f - ANGLE_DELTA;
#define CHECK_YAW(y) while (y < 0.0f) y += 2 * M_PI; while (y >= 2 * M_PI) y -= 2 * M_PI;

// include the Direct3D Library files
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

// global declarations
LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;    // the pointer to the device class
LPDIRECT3DVERTEXBUFFER9 terrainBuffer;
LPD3DXFONT font;
D3DLIGHT9 light;
D3DMATERIAL9 material;
float curYaw = 0.0f;
float curPitch = (float)M_PI / 8.0f;
float curScale = 1.0f;

// function prototypes
void initD3D(HWND hWnd);    // sets up and initializes Direct3D
void render_frame(void);    // renders a single frame
void cleanD3D(void);    // closes Direct3D and releases memory
void init_graphics(void);    // 3D declarations

struct CUSTOMVERTEX {FLOAT X, Y, Z; D3DXVECTOR3 NORMAL; DWORD COLOR;};
#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL)

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	               HINSTANCE hPrevInstance,
	               LPSTR lpCmdLine,
	               int nCmdShow)
{
	HWND hWnd;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	srand(time(NULL));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = "WindowClass";

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(NULL, "WindowClass", "Direct3D",
	                      WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
	                      NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);

	// set up and initialize Direct3D
	initD3D(hWnd);

	// enter the main loop:

	MSG msg;

	while(TRUE)
	{
	    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	    {
	        TranslateMessage(&msg);
	        DispatchMessage(&msg);
	    }

	    if(msg.message == WM_QUIT)
	        break;

	    render_frame();
	}

	// clean up DirectX and COM
	cleanD3D();

	return msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	    case WM_DESTROY:
	        PostQuitMessage(0);
	        return 0;       
		case WM_KEYDOWN:
			switch (wParam) {
				case VK_ESCAPE: case 'q': case 'Q':
					DestroyWindow(hWnd);
					break;
				case VK_RIGHT:
					curYaw -= ANGLE_DELTA;
					CHECK_YAW(curYaw);
					break;
				case VK_LEFT:
					curYaw += ANGLE_DELTA;
					CHECK_YAW(curYaw);
					break;
				case VK_UP:
					curPitch += ANGLE_DELTA;					
					CHECK_PITCH(curPitch);
					break;
				case VK_DOWN:
					curPitch -= ANGLE_DELTA;
					CHECK_PITCH(curPitch);
					break;
				case VK_NEXT:
					curScale *= 1.1f;
					break;
				case VK_PRIOR:
					curScale /= 1.1f;
					break;
			}		
			return 0;		
	}

	return DefWindowProc (hWnd, message, wParam, lParam);
}

void setAA(D3DPRESENT_PARAMETERS& d3dpp) {
	DWORD qual;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	for (int i = D3DMULTISAMPLE_16_SAMPLES; i > D3DMULTISAMPLE_NONMASKABLE; i--) {
		if (SUCCEEDED(d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, TRUE, (D3DMULTISAMPLE_TYPE)i, &qual))) {
			d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE)i;
			d3dpp.MultiSampleQuality = qual - 1;
			return;
		}
	}
}


// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	D3DPRESENT_PARAMETERS d3dpp;

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;    
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferWidth = SCREEN_WIDTH;
	d3dpp.BackBufferHeight = SCREEN_HEIGHT;	
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.EnableAutoDepthStencil = TRUE;

	setAA(d3dpp);

	// create a device class using this information and the info from the d3dpp stuct
	HRESULT r = d3d->CreateDevice(D3DADAPTER_DEFAULT,
	                  D3DDEVTYPE_HAL,
	                  hWnd,
	                  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
	                  &d3dpp,
	                  &d3ddev);

	init_graphics();    // call the function to initialize the triangle

	d3ddev->SetRenderState(D3DRS_LIGHTING, TRUE);
	d3ddev->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	d3ddev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);	
	//d3ddev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	d3ddev->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(31, 31, 31));

	ZeroMemory(&light, sizeof(light));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = light.Diffuse.g = light.Diffuse.b = light.Diffuse.a = 0.75f;	
	light.Direction.x = -1.0f;
	light.Direction.z = 1.0f;
	light.Direction.y = 0.0f;
	light.Range = 1000.0f;

	d3ddev->SetLight(0, &light);
	d3ddev->LightEnable(0, TRUE);
}


// this is the function used to render a single frame
void render_frame(void)
{
	static const int NBUKKIT = 200;
	static ULARGE_INTEGER bukkits[NBUKKIT];
	static int bukidx = 0;	
	int nbuk = (bukidx + 1) % NBUKKIT;
	int pbuk = (bukidx + NBUKKIT - 1) % NBUKKIT;
	int ntick = 0;	
	FILETIME curFrame;	

	GetSystemTimeAsFileTime(&curFrame);
	bukkits[bukidx].HighPart = curFrame.dwHighDateTime;
	bukkits[bukidx].LowPart = curFrame.dwLowDateTime;

	if (bukkits[pbuk].QuadPart != 0) {
		ntick = bukkits[bukidx].LowPart / 10000 - bukkits[pbuk].LowPart / 10000;
	}

	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	d3ddev->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	d3ddev->BeginScene();

	// select which vertex format we are using
	d3ddev->SetFVF(CUSTOMFVF);

	RECT r;
	r.left = 0; 
	r.top = 0;
	r.bottom = 0;
	r.right = 0;

	
	char buff[1024];
	if (bukkits[nbuk].QuadPart != 0) {		
		double fps = (float)NBUKKIT * (10000000.0 / (double)(bukkits[bukidx].QuadPart - bukkits[nbuk].QuadPart));
		sprintf_s(buff, 1024, "FPS: %0.1f", fps);
	} else {
		strcpy_s(buff, 1024, "FPS: ??");
	}
	font->DrawText(NULL, buff, -1, &r, DT_CALCRECT, D3DCOLOR_XRGB(1, 1, 1));
	int h = font->DrawText(NULL, buff, -1, &r, 0, D3DCOLOR_XRGB(0, 255, 0));

	sprintf_s(buff, 1024, "Pitch: %0.3f\nYaw  : %0.3f\nScale: %0.3f", curPitch, curYaw, curScale);
	r.left = 0;
	r.top = h;
	r.bottom = h;
	r.right = 0;
	font->DrawTextA(NULL, buff, -1, &r, DT_CALCRECT, D3DCOLOR_XRGB(0, 0, 0));
	font->DrawTextA(NULL, buff, -1, &r, 0, D3DCOLOR_XRGB(0, 255, 0));

	material.Diffuse.r = material.Diffuse.g = material.Diffuse.b = material.Diffuse.a = 1.0f;
	material.Ambient = material.Diffuse;
	d3ddev->SetMaterial(&material);

	// SET UP THE PIPELINE    

	D3DXMATRIX matView;    // the view transform matrix	
	D3DXMATRIX matRotateYaw;
	D3DXMATRIX matRotatePitch;
	D3DXMATRIX matScale;
	D3DXMatrixRotationX(&matRotatePitch, -curPitch);
	D3DXMatrixRotationZ(&matRotateYaw, curYaw);
	D3DXMatrixScaling(&matScale, curScale, curScale, curScale);

	D3DXVECTOR4 vecCamera;
	D3DXVec3Transform(&vecCamera, &D3DXVECTOR3(0.0f, -15.0f, 0.0f), &(matRotatePitch * matRotateYaw * matScale));

	D3DXMatrixLookAtLH(&matView,
	                   &D3DXVECTOR3(vecCamera.x, vecCamera.y, vecCamera.z),    // the camera position
	                   &D3DXVECTOR3 (0.0f, 0.0f, 0.0f),    // the look-at position
	                   &D3DXVECTOR3 (0.0f, 0.0f, 1.0f));    // the up direction

	d3ddev->SetTransform(D3DTS_VIEW, &matView);    // set the view transform to matView	

	D3DXMATRIX matProjection;     // the projection transform matrix

	D3DXMatrixPerspectiveFovLH(&matProjection,
	                           D3DXToRadian(45),    // the horizontal field of view
	                           (FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, // aspect ratio
	                           1.0f,    // the near view-plane
	                           100.0f);    // the far view-plane

	d3ddev->SetTransform(D3DTS_PROJECTION, &matProjection);    // set the projection	

	D3DXMATRIX matIdentity;
	D3DXMatrixIdentity(&matIdentity);
	d3ddev->SetTransform(D3DTS_WORLD, &matIdentity);
	
	d3ddev->SetStreamSource(0, terrainBuffer, 0, sizeof(CUSTOMVERTEX));	
	
	d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, T_STEPS * T_STEPS * 2);

	d3ddev->EndScene();

	d3ddev->Present(NULL, NULL, NULL, NULL);

	bukidx++;
	bukidx %= NBUKKIT;
}


// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{	
	terrainBuffer->Release();
	font->Release();
	d3ddev->Release();    // close and release the 3D device
	d3d->Release();    // close and release Direct3D
}


// this is the function that puts the 3D models into video RAM
void init_graphics(void)
{   
	HRESULT hr = D3DXCreateFont(d3ddev, 17, 0, FW_NORMAL, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Courier New"), &font );

	d3ddev->CreateVertexBuffer(
		(T_STEPS) * (T_STEPS) * 3 * 2 * sizeof(CUSTOMVERTEX),
		D3DUSAGE_WRITEONLY,
		CUSTOMFVF,
		D3DPOOL_MANAGED,
		&terrainBuffer,
		NULL);

	Terrain::Terrain t;
#undef T_GENFUNC
#define T_GENFUNC t.height
	CUSTOMVERTEX* vertices;
	unsigned index = 0;
	terrainBuffer->Lock(0, 0, (void**)&vertices, 0);
	for (unsigned i = 0; i < T_STEPS; i++) {
		float y = T_Y(i);
		for (unsigned j = 0; j < T_STEPS; j++) {
			float x = T_X(j);		
			CUSTOMVERTEX* cur = &vertices[index];
			cur[0].X = x;
			cur[0].Y = y;
			cur[0].Z = T_GENFUNC(x, y);
			cur[1].X = T_X(j + 1);
			cur[1].Y = y;
			cur[1].Z = T_GENFUNC(T_X(j + 1), y);
			cur[2].X = x;
			cur[2].Y = T_Y(i + 1);
			cur[2].Z = T_GENFUNC(x, T_Y(i + 1));
			cur[3] = cur[2];
			cur[4] = cur[1];
			cur[5].X = T_X(j + 1);
			cur[5].Y = T_Y(i + 1);
			cur[5].Z = T_GENFUNC(T_X(j+1),T_Y(i+1));
			cur[0].COLOR = cur[1].COLOR = cur[2].COLOR = cur[3].COLOR = cur[4].COLOR = cur[5].COLOR = D3DCOLOR_XRGB(255, 255, 255);

			{
			D3DXVECTOR3 a(cur[0].X, cur[0].Y, cur[0].Z), b(cur[1].X, cur[1].Y, cur[1].Z), c(cur[2].X, cur[2].Y, cur[2].Z);
			D3DXVECTOR3 n;
			D3DXVec3Cross(&n, &(b-c), &(a-c));
			D3DXVec3Normalize(&n, &n);
			cur[0].NORMAL = cur[1].NORMAL = cur[2].NORMAL = n;
			}

			{
			D3DXVECTOR3 a(cur[3].X, cur[3].Y, cur[3].Z), b(cur[4].X, cur[4].Y, cur[4].Z), c(cur[5].X, cur[5].Y, cur[5].Z);
			D3DXVECTOR3 n;
			D3DXVec3Cross(&n, &(b-c), &(a-c));
			D3DXVec3Normalize(&n, &n);
			cur[3].NORMAL = cur[4].NORMAL = cur[5].NORMAL = n;
			}

			index += 6;
		}
	}
	terrainBuffer->Unlock();
}
