// include the basic windows header files and the Direct3D header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>

#include <cmath>
#include <cstdio>

#undef M_PI
#define M_PI 3.141592653589793238462643 

// define the screen resolution
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define T_WIDTH		8.0f
#define T_HEIGHT	8.0f
#define T_STEPS		64

#define T_OFF(x,y) ((y) * ((T_STEPS) + 1) + (x))

#define ANGLE_DELTA (float)(M_PI / 32.0f)

// include the Direct3D Library files
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

// global declarations
LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;    // the pointer to the device class
LPDIRECT3DVERTEXBUFFER9 terrainBuffer;
LPDIRECT3DINDEXBUFFER9	indexBuffer;
LPD3DXFONT font;
float curYaw = 0.0f;
float curPitch = 0.0f;

// function prototypes
void initD3D(HWND hWnd);    // sets up and initializes Direct3D
void render_frame(void);    // renders a single frame
void cleanD3D(void);    // closes Direct3D and releases memory
void init_graphics(void);    // 3D declarations

struct CUSTOMVERTEX {FLOAT X, Y, Z; DWORD COLOR;};
#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)

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
				case VK_LEFT:
					curYaw -= ANGLE_DELTA;
					break;
				case VK_RIGHT:
					curYaw += ANGLE_DELTA;
					break;
				case VK_UP:
					curPitch += ANGLE_DELTA;
					break;
				case VK_DOWN:
					curPitch -= ANGLE_DELTA;
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

	d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
	//d3ddev->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	d3ddev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);	
	d3ddev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
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

	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0);
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
	font->DrawText(NULL, buff, -1, &r, 0, D3DCOLOR_XRGB(0, 255, 0));

	// SET UP THE PIPELINE    

	D3DXMATRIX matView;    // the view transform matrix

	D3DXMatrixLookAtLH(&matView,
	                   &D3DXVECTOR3 (0.0f, 0.0f, -15.0f),    // the camera position
	                   &D3DXVECTOR3 (0.0f, 0.0f, 0.0f),    // the look-at position
	                   &D3DXVECTOR3 (0.0f, 1.0f, 0.0f));    // the up direction

	d3ddev->SetTransform(D3DTS_VIEW, &matView);    // set the view transform to matView

	D3DXMATRIX matProjection;     // the projection transform matrix

	D3DXMatrixPerspectiveFovLH(&matProjection,
	                           D3DXToRadian(45),    // the horizontal field of view
	                           (FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, // aspect ratio
	                           1.0f,    // the near view-plane
	                           100.0f);    // the far view-plane

	d3ddev->SetTransform(D3DTS_PROJECTION, &matProjection);    // set the projection
	
	D3DXMATRIX matRotateYaw;
	D3DXMATRIX matRotatePitch;
	D3DXMatrixRotationX(&matRotatePitch, curPitch);
	D3DXMatrixRotationZ(&matRotateYaw, curYaw);

	d3ddev->SetTransform(D3DTS_WORLD, &(matRotateYaw * matRotatePitch));	

	CUSTOMVERTEX* vertices;
	terrainBuffer->Lock(0, 0, (void**)&vertices, 0);
	for (unsigned i = 0; i < T_STEPS + 1; i++) {
		float y = -(T_HEIGHT / 2.0f) + i * T_HEIGHT / T_STEPS;
		for (unsigned j = 0; j < T_STEPS + 1; j++) {
			float x = -(T_WIDTH / 2.0f) + j * T_WIDTH / T_STEPS;
			CUSTOMVERTEX& cur = vertices[T_OFF(j,i)];
			cur.X = x;
			cur.Y = y;
			cur.Z = sin((x+y)*M_PI/2.0) / 2.0f;
			cur.COLOR = D3DCOLOR_XRGB(0, 0, 0);
		}
	}
	terrainBuffer->Unlock();
	
	d3ddev->SetStreamSource(0, terrainBuffer, 0, sizeof(CUSTOMVERTEX));
	for (unsigned row = 0; row < T_STEPS; row++) {
		short* indices;
		indexBuffer->Lock(0, 0, (void**)&indices, 0);
		indices[0] = T_OFF(0, row);
		indices[1] = T_OFF(0, row + 1);
		
		for (unsigned col = 1; col < T_STEPS + 1; col++) {
			indices[col * 2] = T_OFF(col, row);
			indices[col * 2 + 1] = T_OFF(col, row + 1);
		}
		indexBuffer->Unlock();
		d3ddev->SetIndices(indexBuffer);
		d3ddev->DrawIndexedPrimitive(
			D3DPT_TRIANGLESTRIP,
			0, 
			0,
			(T_STEPS + 1) * (T_STEPS + 1),
			0, 
			T_STEPS * 2);
	}

	d3ddev->EndScene();

	d3ddev->Present(NULL, NULL, NULL, NULL);

	bukidx++;
	bukidx %= NBUKKIT;
}


// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
	indexBuffer->Release();
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
		(T_STEPS + 1) * (T_STEPS + 1) * sizeof(CUSTOMVERTEX),
		D3DUSAGE_WRITEONLY,
		CUSTOMFVF,
		D3DPOOL_MANAGED,
		&terrainBuffer,
		NULL);
	d3ddev->CreateIndexBuffer(
		128 * sizeof(short),
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&indexBuffer,
		NULL);

}
