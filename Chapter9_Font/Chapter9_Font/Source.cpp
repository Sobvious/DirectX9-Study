#include <Windows.h>
#include <d3dx9.h>

LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, INT nCmdShow) {

	// Declare variables
	const char* className = "Window Class";
	const char* caption = "Window";
	const int window_width = 640;
	const int window_height = 480;

	// Register Window Class
	WNDCLASSEX wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = MsgProc;
	wndClass.hInstance = hInstance;
	wndClass.lpszClassName = (LPSTR)className;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	RegisterClassEx(&wndClass);

	// Create and Show Window
	HWND hWnd = CreateWindow(className, caption, WS_OVERLAPPEDWINDOW, 0, 0, window_width, window_height, NULL, NULL, wndClass.hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	// Init Direct3D9 for Creating Device
	IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	D3DDEVTYPE deviceType = D3DDEVTYPE_HAL;
	int vertexProcessing = 0;

	// Graphic card check for Creating Device
	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, deviceType, &caps);

	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
		vertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else {
		vertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	// Presentation Parameters for Creating Device
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = window_width;
	d3dpp.BackBufferHeight = window_height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.Windowed = true;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	// Create Device
	LPDIRECT3DDEVICE9 device;
	d3d9->CreateDevice(D3DADAPTER_DEFAULT, deviceType, hWnd, vertexProcessing, &d3dpp, &device);
	d3d9->Release();

	// Rendering Pipeline (You don't have to write these with this order)

	// Local -> World Space
	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity(&matWorld);
	device->SetTransform(D3DTS_WORLD, &matWorld);

	// View Space
	D3DXMATRIXA16 matView;
	D3DXVECTOR3 vecEye(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vecLookAt(1.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vecUp(0.0f, 0.0f, 1.0f);

	D3DXMatrixLookAtLH(&matView, &vecEye, &vecLookAt, &vecUp);

	device->SetTransform(D3DTS_VIEW, &matView);

	// Backface Culling
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);//default setting

	// Projection
	D3DXMATRIXA16 matProj;

	D3DXMatrixPerspectiveFovLH(&matProj, 74.0f, (float)window_width / window_height, 0.1f, 32000.0f);

	device->SetTransform(D3DTS_PROJECTION, &matProj);

	// Viewport Transform
	D3DVIEWPORT9 vp = { 0, 0, window_width, window_height, 0, 1 };
	device->SetViewport(&vp);

	// *Font
	ID3DXFont* font = nullptr;

	D3DXFONT_DESCA lf;  
	::ZeroMemory(&lf, sizeof(lf));
	lf.Height = 25;
	lf.Width = 12;
	lf.Weight = 500;
	lf.CharSet = DEFAULT_CHARSET;
	strcpy(lf.FaceName, "Time new Roman");

	D3DXCreateFontIndirect(device, &lf, &font);

	RECT text_area;
	text_area.left = 0;
	text_area.top = 0;
	text_area.right = 256;
	text_area.bottom = 64;

	// Message Loop
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT) {

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// Present
			device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0.0f);
			device->BeginScene();

			font->DrawText(NULL, "Hello, world!", -1, &text_area, DT_TOP | DT_LEFT, 0xff00ff00);

			device->EndScene();
			device->Present(NULL, NULL, NULL, NULL);
		}

	}

	// Release
	UnregisterClass((LPSTR)className, wndClass.hInstance);
	device->Release();

	return 0;
}

LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}