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
	D3DXVECTOR3 vecEye(24.0f, 0.0f, 0.0f);
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

	// *Model
	ID3DXMesh* model = 0;
	std::vector<IDirect3DTexture9*> model_textures;
	std::vector<D3DMATERIAL9> model_materials;

	ID3DXBuffer* buffer_adj = 0;
	ID3DXBuffer* buffer_mtrl = 0;
	DWORD number_mtrls = 0;

	D3DXLoadMeshFromX("bigship1.x", D3DXMESH_MANAGED, device, &buffer_adj, &buffer_mtrl, 0, &number_mtrls, &model);

	D3DXMATERIAL* mtrls = (D3DXMATERIAL*) buffer_mtrl->GetBufferPointer();
	
	for (int i = 0; i < number_mtrls; i++) {
		// *Add Materials
		mtrls[i].MatD3D.Ambient = mtrls[i].MatD3D.Diffuse;
		model_materials.push_back(mtrls[i].MatD3D);

		// *Add Textures
		IDirect3DTexture9* tex = 0;
		if (mtrls[i].pTextureFilename != 0) {
			D3DXCreateTextureFromFile(device, mtrls[i].pTextureFilename, &tex);
		}
		model_textures.push_back(tex);
	}

	buffer_mtrl->Release();

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

	// Device Set
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

			// *Draw Model
			for (int i = 0; i < model_materials.size(); i++) {
				device->SetMaterial(&model_materials[i]);
				device->SetTexture(0, model_textures[i]);
				model->DrawSubset(i);
			}

			// End
			device->EndScene();
			device->Present(NULL, NULL, NULL, NULL);
		}

	}

	// *Release
	UnregisterClass((LPSTR)className, wndClass.hInstance);
	device->Release();
	model->Release();
	for (int i = 0; i < model_materials.size(); i++) {
		if (model_textures[i] != 0)
		model_textures[i]->Release();
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