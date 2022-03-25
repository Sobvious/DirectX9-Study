#include <Windows.h>
#include <d3dx9.h>
#include <timeapi.h>
#include <vector>

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
	D3DXVECTOR3 vecEye(3.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vecLookAt(0.0f, 0.0f, 0.0f);
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

	// *Cube
	ID3DXMesh* cube;
	D3DXCreateMeshFVF(12, 24, D3DXMESH_MANAGED, Vertex::FVF, device, &cube);

		// *Vertex Buffer
	Vertex* vertices;
	cube->LockVertexBuffer(0, (void**)&vertices);
	// z+
	vertices[0] = Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255), 0.0f, 0.0f);
	vertices[1] = Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255), 0.0f, 1.0f);
	vertices[2] = Vertex(1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 1.0f);
	vertices[3] = Vertex(1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0.0f);
	// z-
	vertices[4] = Vertex(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 255, 255), 0.0f, 0.0f);
	vertices[5] = Vertex(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 255, 255), 0.0f, 1.0f);
	vertices[6] = Vertex(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 1.0f);
	vertices[7] = Vertex(1.0f, -1.0f, 1.0f, 0.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0.0f);
	// y-
	vertices[8] = Vertex(-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 0.0f, 0.0f);
	vertices[9] = Vertex(-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 0.0f, 1.0f);
	vertices[10] = Vertex(1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 1.0f);
	vertices[11] = Vertex(1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0.0f);
	// y+
	vertices[12] = Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 0.0f, 0.0f);
	vertices[13] = Vertex(-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 0.0f, 1.0f);
	vertices[14] = Vertex(1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 1.0f);
	vertices[15] = Vertex(1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0.0f);
	// x-
	vertices[16] = Vertex(-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 0.0f, 0.0f);
	vertices[17] = Vertex(-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 0.0f, 1.0f);
	vertices[18] = Vertex(-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 1.0f);
	vertices[19] = Vertex(-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0.0f);
	// x+
	vertices[20] = Vertex(1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 0.0f, 0.0f);
	vertices[21] = Vertex(1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 0.0f, 1.0f);
	vertices[22] = Vertex(1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 1.0f);
	vertices[23] = Vertex(1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0.0f);

	// *Index Buffer
	cube->UnlockVertexBuffer();

	WORD* indices = 0;
	cube->LockIndexBuffer(0, (void**)&indices);
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 0; indices[4] = 2; indices[5] = 3;

	indices[6] = 4; indices[7] = 6; indices[8] = 5;
	indices[9] = 4; indices[10] = 7; indices[11] = 6;

	indices[12] = 8; indices[13] = 10; indices[14] = 9;
	indices[15] = 8; indices[16] = 11; indices[17] = 10;

	indices[18] = 12; indices[19] = 13; indices[20] = 14;
	indices[21] = 12; indices[22] = 14; indices[23] = 15;

	indices[24] = 16; indices[25] = 17; indices[26] = 18;
	indices[27] = 16; indices[28] = 18; indices[29] = 19;

	indices[30] = 20; indices[31] = 22; indices[32] = 21;
	indices[33] = 20; indices[34] = 23; indices[35] = 22;
	cube->UnlockIndexBuffer();

		// *Attribute Buffer
	DWORD* attributeBuffer = 0;
	cube->LockAttributeBuffer(0, &attributeBuffer);
	for (int a = 0; a < 4; a++)
		attributeBuffer[a] = 0;
	for (int a = 4; a < 8; a++)
		attributeBuffer[a] = 1;
	for (int a = 8; a < 12; a++)
		attributeBuffer[a] = 2;
	cube->UnlockAttributeBuffer();

		// *adjacency Buffer
	std::vector<DWORD> adjacencyBuffer(cube->GetNumFaces() * 3);
	cube->GenerateAdjacency(0.0f, &adjacencyBuffer[0]);

	cube->OptimizeInplace(
		D3DXMESHOPT_ATTRSORT |
		D3DXMESHOPT_COMPACT |
		D3DXMESHOPT_VERTEXCACHE,
		&adjacencyBuffer[0],
		0,0,0
	);

		// *Texture
	IDirect3DTexture9* tex[3] = {0, 0, 0};
	
	D3DXCreateTextureFromFile(device, "checker.jpg", &tex[0]);
	D3DXCreateTextureFromFile(device, "brick0.jpg", &tex[1]);
	D3DXCreateTextureFromFile(device, "brick1.jpg", &tex[2]);


	// Material
	D3DMATERIAL9 material;
	::ZeroMemory(&material, sizeof(material));
	material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	material.Specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	material.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	material.Power = 5.0f;
	device->SetMaterial(&material);

	// Directional Light
	D3DLIGHT9 light;
	::ZeroMemory(&light, sizeof(light));

	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Ambient = D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.3f);
	light.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	light.Specular = D3DXCOLOR(0.6f, 0.6f, 0.6f, 0.6f);
	light.Direction = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
	device->SetLight(0, &light);
	device->LightEnable(0, true);

	// *Device Set
	device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	device->SetRenderState(D3DRS_LIGHTING, true);
	device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	device->SetRenderState(D3DRS_SPECULARENABLE, true);

	device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	device->SetFVF(Vertex::FVF);

	// Rotation Transform
	D3DXMATRIXA16 matRotationX, matRotationZ, matRotationResult;
	float angle = 0.0f;

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

			// Begin
			device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0.0f);
			device->BeginScene();

			// Rotation Transform
			angle += D3DX_PI / 4.0f * deltaTime;
			D3DXMatrixRotationX(&matRotationX, angle);
			D3DXMatrixRotationZ(&matRotationZ, angle);
			matRotationResult = matRotationX * matRotationZ;
			device->SetTransform(D3DTS_WORLD, &matRotationResult);

			// *Draw Cube
			for ( int i = 0; i < 3; i ++ ) {
				device->SetTexture(0, tex[i]);
				cube->DrawSubset(i);
			}

			// End
			device->EndScene();
			device->Present(NULL, NULL, NULL, NULL);
		}

	}

	// *Release
	UnregisterClass((LPSTR)className, wndClass.hInstance);
	device->Release();
	cube->Release();
	for (int i = 0; i < 3; i++) {
		tex[i]->Release();
	}
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