#include <Windows.h>
#include <d3dx9.h>
#include <timeapi.h>
#include <vector>

// Macro
#define LANDOBJECT 0
#define AIRCRAFT 1

LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Vertex Struct
struct Vertex {
	Vertex() {};
	Vertex(float x, float y, float  z, float nx, float ny, float nz, DWORD color, float u, float v) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->nx = nx;
		this->ny = ny;
		this->nz = nz;
		this->color = color;
		this->u = u;
		this->v = v;
	}
	float x, y, z, nx, ny, nz;
	DWORD color;
	float u, v;
	static const DWORD FVF;
};
const DWORD Vertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1;


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, INT nCmdShow) {

	// Declare variables
	const char* className = "Window Class";
	const char* caption = "Direct3D9";
	const int window_width = 640;
	const int window_height = 480;
	float timePrevious = 0.0f;
	float deltaTime = 0.0f;
	bool cameraType = AIRCRAFT;

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

	// Local -> World Space
	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity(&matWorld);
	device->SetTransform(D3DTS_WORLD, &matWorld);

	// View Space(Camera)
	D3DXMATRIXA16 matView;
	D3DXVECTOR3 vecEye(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vecLookAt(1.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vecUp(0.0f, 0.0f, 1.0f);

	D3DXVECTOR3 vecRight(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 vecForward(1.0f, 0.0f, 0.0f);

	// Backface Culling
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);//default setting

	// Projection
	D3DXMATRIXA16 matProj;

	D3DXMatrixPerspectiveFovLH(&matProj, -74.0f, (float)window_width / window_height, 0.1f, 32000.0f);

	device->SetTransform(D3DTS_PROJECTION, &matProj);

	// Viewport Transform
	D3DVIEWPORT9 vp = { 0, 0, window_width, window_height, 0, 1 };
	device->SetViewport(&vp);

	// Terrain
	ID3DXMesh* terrain;
	

	// *Read Height Map
	IDirect3DTexture9* heightMap;
	D3DXCreateTextureFromFile(device, "Heightmap_top.bmp", &heightMap);

	D3DSURFACE_DESC heightMapDesc;
	heightMap->GetLevelDesc(0, &heightMapDesc);

	D3DXCreateMeshFVF((heightMapDesc.Height - 1)*(heightMapDesc.Width - 1)*2, (heightMapDesc.Height) * (heightMapDesc.Width), D3DXMESH_MANAGED, Vertex::FVF, device, &terrain);

	// *Vertex Buffer from Height Map
	Vertex *vertices;
	D3DLOCKED_RECT lockedRect;

	terrain->LockVertexBuffer(0, (void**)&vertices);
	heightMap->LockRect(0, &lockedRect, 0, 0);
	unsigned int* imageData = (unsigned int*)lockedRect.pBits;
	for (int i = 0; i < heightMapDesc.Height; i++) {
		for (int j = 0; j < heightMapDesc.Width; j++) {
			int index = i * heightMapDesc.Width + j;
			unsigned char data = imageData[index]&0xFF;
			
			vertices[index] = Vertex(j, i, 
				(float)data/50.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255), 
				(float)j/(float)heightMapDesc.Width, (float)i / (float)heightMapDesc.Height);
		}
	}
	heightMap->UnlockRect(0);
	terrain->UnlockVertexBuffer();


	// *Index Buffer
	WORD* indices = 0;
	int indices_index = 0;
	terrain->LockIndexBuffer(0, (void**)&indices);
	heightMap->LockRect(0, &lockedRect, 0, 0);
	for (int i = 0; i < heightMapDesc.Height-1; i++) {
		for (int j = 0; j < heightMapDesc.Width-1; j++) {
			int index = i * heightMapDesc.Width + j;

			indices[indices_index++] = index;		indices[indices_index++] = index + 1;	indices[indices_index++] = index + heightMapDesc.Width;
			indices[indices_index++] = index + 1;	indices[indices_index++] = index + heightMapDesc.Width + 1;	indices[indices_index++] = index + heightMapDesc.Width;
		}
	}
	heightMap->UnlockRect(0);
	terrain->UnlockIndexBuffer();

	// *Attribute Buffer
	DWORD* attributeBuffer = 0;
	terrain->LockAttributeBuffer(0, &attributeBuffer);
	for (int a = 0; a < (heightMapDesc.Height - 1)*(heightMapDesc.Width - 1); a++) attributeBuffer[a] = 0;
	terrain->UnlockAttributeBuffer();

	// Texture
	IDirect3DTexture9* tex = 0;
	D3DXCreateTextureFromFile(device, "desert.bmp", &tex);

	// Device Set
	device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	device->SetRenderState(D3DRS_LIGHTING, false);
	device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	device->SetFVF(Vertex::FVF);


	// Message Loop
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT) {

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// DeltaTime
			deltaTime = (timeGetTime() - timePrevious) * 0.001f;
			timePrevious = timeGetTime();

			// Camera
			if (GetAsyncKeyState('W') & 0x8000) {
				float units = 4.0f;
				// Walk
				if (cameraType == LANDOBJECT) {
					D3DXVECTOR3 vecFront = D3DXVECTOR3(vecForward.x, vecForward.y, 0.0f);
					D3DXVec3Normalize(&vecFront, &vecFront);
					vecEye += vecFront * units * deltaTime;
				}
				if (cameraType == AIRCRAFT) {
					vecEye += vecForward * units * deltaTime;
				}
			}
			if (GetAsyncKeyState('S') & 0x8000) {
				float units = -4.0f;
				// Walk
				if (cameraType == LANDOBJECT) {
					D3DXVECTOR3 vecFront = D3DXVECTOR3(vecForward.x, vecForward.y, 0.0f);
					D3DXVec3Normalize(&vecFront, &vecFront);
					vecEye += vecFront * units * deltaTime;
				}
				if (cameraType == AIRCRAFT) {
					vecEye += vecForward * units * deltaTime;
				}
			}
			if (GetAsyncKeyState('D') & 0x8000) {
				float units = 4.0f;
				// Strafe
				if (cameraType == LANDOBJECT) {
					vecRight = D3DXVECTOR3(vecRight.x, vecRight.y, 0.0f);
					D3DXVec3Normalize(&vecRight, &vecRight);
					vecEye += vecRight * units * deltaTime;
				}
				if (cameraType == AIRCRAFT) {
					vecEye += vecRight * units * deltaTime;
				}
			}
			if (GetAsyncKeyState('A') & 0x8000) {
				float units = -4.0f;
				// Strafe
				if (cameraType == LANDOBJECT) {
					vecRight = D3DXVECTOR3(vecRight.x, vecRight.y, 0.0f);
					D3DXVec3Normalize(&vecRight, &vecRight);
					vecEye += vecRight * units * deltaTime;
				}
				if (cameraType == AIRCRAFT) {
					vecEye += vecRight * units * deltaTime;
				}
			}
			if (GetAsyncKeyState('F') & 0x8000) {
				float units = 4.0f;
				// Fly
				if (cameraType == LANDOBJECT) {
					vecEye.z += units * deltaTime;
				}
				if (cameraType == AIRCRAFT) {
					vecEye += vecUp * units * deltaTime;
				}
			}
			if (GetAsyncKeyState('R') & 0x8000) {
				float units = -4.0f;
				// Fly
				if (cameraType == LANDOBJECT) {
					vecEye.z += units * deltaTime;
				}
				if (cameraType == AIRCRAFT) {
					vecEye += vecUp * units * deltaTime;
				}
			}
			if (GetAsyncKeyState(VK_UP)) {
				float units = -1.0f;
				// Pitch
				D3DXMATRIXA16 T;
				D3DXMatrixRotationAxis(&T, &vecRight, units * deltaTime);
				D3DXVec3TransformCoord(&vecForward, &vecForward, &T);
				D3DXVec3TransformCoord(&vecUp, &vecUp, &T);
			}
			if (GetAsyncKeyState(VK_DOWN)) {
				float units = 1.0f;
				// Pitch
				D3DXMATRIXA16 T;
				D3DXMatrixRotationAxis(&T, &vecRight, units * deltaTime);
				D3DXVec3TransformCoord(&vecForward, &vecForward, &T);
				D3DXVec3TransformCoord(&vecUp, &vecUp, &T);
			}
			if (GetAsyncKeyState(VK_RIGHT)) {
				float units = 4.0f;
				// Yaw
				D3DXMATRIXA16 T;
				if (cameraType == LANDOBJECT) {
					D3DXMatrixRotationZ(&T, units * deltaTime);
				}
				if (cameraType == AIRCRAFT) {
					D3DXMatrixRotationAxis(&T, &vecUp, units * deltaTime);
				}
				D3DXVec3TransformCoord(&vecForward, &vecForward, &T);
				D3DXVec3TransformCoord(&vecRight, &vecRight, &T);
			}
			if (GetAsyncKeyState(VK_LEFT)) {
				float units = -4.0f;
				// Yaw
				D3DXMATRIXA16 T;
				if (cameraType == LANDOBJECT) {
					D3DXMatrixRotationZ(&T, units * deltaTime);
				}
				if (cameraType == AIRCRAFT) {
					D3DXMatrixRotationAxis(&T, &vecUp, units * deltaTime);
				}
				D3DXVec3TransformCoord(&vecForward, &vecForward, &T);
				D3DXVec3TransformCoord(&vecRight, &vecRight, &T);
			}
			if (GetAsyncKeyState('N')) {
				float units = 4.0f;
				// Roll
				D3DXMATRIXA16 T;
				D3DXMatrixRotationAxis(&T, &vecForward, units * deltaTime);
				D3DXVec3TransformCoord(&vecRight, &vecRight, &T);
				D3DXVec3TransformCoord(&vecUp, &vecUp, &T);
			}
			if (GetAsyncKeyState('M')) {
				float units = -4.0f;
				// Roll
				D3DXMATRIXA16 T;
				D3DXMatrixRotationAxis(&T, &vecForward, units * deltaTime);
				D3DXVec3TransformCoord(&vecRight, &vecRight, &T);
				D3DXVec3TransformCoord(&vecUp, &vecUp, &T);
			}
			vecLookAt = vecEye + vecForward;

			D3DXMatrixLookAtLH(&matView, &vecEye, &vecLookAt, &vecUp);
			device->SetTransform(D3DTS_VIEW, &matView);


			// Begin
			device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0.0f);
			device->BeginScene();

			// Draw Terrain
			device->SetTexture(0, tex);
			terrain->DrawSubset(0);

			// End
			device->EndScene();
			device->Present(NULL, NULL, NULL, NULL);
		}

	}

	// Release
	UnregisterClass((LPSTR)className, wndClass.hInstance);
	device->Release();
	terrain->Release();
	tex->Release();
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