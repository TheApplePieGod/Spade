#include "DX11Renderer.h"
#include "../../Engine.h"
#include "AssetUtils.h"

extern engine* Engine;
extern shader_constants_actor ActorConstants;
extern shader_constants_frame FrameConstants;
extern shader_constants_material MaterialConstants;
extern shader_constants_lighting LightingConstants;

pipeline_state CurrentState = pipeline_state();

void dx11_renderer::Initialize(void* _Window, int WindowWidth, int WindowHeight)
{
	Window = (HWND)_Window;

	//
	// Device and swap chain
	//

	D3D_FEATURE_LEVEL FeatureLevelsSupported[] = {
		//D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	UINT createDeviceFlags = 0;
#if SPADE_DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, FeatureLevelsSupported, _countof(FeatureLevelsSupported), D3D11_SDK_VERSION, &Device, NULL, &DeviceContext);
	if (FAILED(hr))
		Assert(1 == 2);

	IDXGIDevice* pDXGIDevice = nullptr;
	hr = Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);

	IDXGIAdapter* pDXGIAdapter = nullptr;
	hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);

	IDXGIFactory* pIDXGIFactory = nullptr;
	hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);
	if (FAILED(hr))
		Assert(1 == 2);

	UINT MultiSamplingCount = 4;
	UINT MaxMultisamplingQuality = 0;
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	Device->CheckMultisampleQualityLevels(Format, MultiSamplingCount, &MaxMultisamplingQuality);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = WindowWidth;
	sd.BufferDesc.Height = WindowHeight;
	sd.BufferDesc.Format = Format;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = Window;
	sd.SampleDesc.Count = MultiSamplingCount;
	sd.SampleDesc.Quality = 0;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Windowed = TRUE;

	hr = pIDXGIFactory->CreateSwapChain(Device, &sd, &Chain);
	if (FAILED(hr))
		Assert(1 == 2);
	
	SAFE_RELEASE(pIDXGIFactory);
	SAFE_RELEASE(pDXGIAdapter);
	SAFE_RELEASE(pDXGIDevice);

	//hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, FeatureLevelsSupported, _countof(FeatureLevelsSupported), D3D11_SDK_VERSION, &sd, &Chain, &Device, NULL, &DeviceContext);
	//if (hr == E_INVALIDARG)
	//{
	//	hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, &FeatureLevelsSupported[1], _countof(FeatureLevelsSupported) - 1, D3D11_SDK_VERSION, &sd, &Chain, &Device, NULL, &DeviceContext);
	//}
	//if (FAILED(hr))
	//	Assert(1 == 2);

#if SPADE_DEBUG

	//
	// Create debug device and supress certain warning messages
	//
	hr = Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&DebugDevice);
	if (FAILED(hr))
		Assert(1 == 2);

	ID3D11InfoQueue* d3dInfoQueue = NULL;
	hr = Device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue);
	if (FAILED(hr))
		Assert(1 == 2);

	D3D11_MESSAGE_ID hide[] =
	{
		// messages to hide
		D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
	};
	D3D11_INFO_QUEUE_FILTER filter;
	ZeroMemory(&filter, sizeof(filter));
	filter.DenyList.NumIDs = _countof(hide);
	filter.DenyList.pIDList = hide;
	d3dInfoQueue->AddStorageFilterEntries(&filter);

	SAFE_RELEASE(d3dInfoQueue);

#endif

	//
	// Create render target and viewport
	//
	ID3D11Texture2D* pRenderTargetTexture;
	hr = Chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pRenderTargetTexture);
	if (FAILED(hr))
		Assert(1 == 2);;

	//D3D11_RENDER_TARGET_VIEW_DESC rtd;
	//rtd.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	hr = Device->CreateRenderTargetView(pRenderTargetTexture, NULL, &RenderTargetView);
	pRenderTargetTexture->Release();
	if (FAILED(hr))
		Assert(1 == 2);

	//DeviceContext->OMSetRenderTargets(1, &RenderTargetView, NULL); //set after depth stencil initialization


	RECT rc;
	GetClientRect(Window, &rc);
	D3D11_VIEWPORT vp;
	ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));
	vp.Width = (FLOAT)(rc.right - rc.left);
	vp.Height = (FLOAT)(rc.bottom - rc.top);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	DeviceContext->RSSetViewports(1, &vp);


	//
	// Rasterizer state
	//
	D3D11_RASTERIZER_DESC rasterDesc;
	// Setup a raster description which turns off back face culling.
	rasterDesc.AntialiasedLineEnable = true;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the no culling rasterizer state.
	hr = Device->CreateRasterizerState(&rasterDesc, &DefaultCullBackface);
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	hr = Device->CreateRasterizerState(&rasterDesc, &DefaultCullFrontface);
	rasterDesc.CullMode = D3D11_CULL_NONE;
	hr = Device->CreateRasterizerState(&rasterDesc, &DefaultCullNone);
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	hr = Device->CreateRasterizerState(&rasterDesc, &Wireframe);
	if (FAILED(hr))
		Assert(1 == 2);

	// compile & register default vertex shader / layout
	{
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

		// VERT SHADER
		ID3DBlob* pBlobError = NULL;
		ID3DBlob* vsBlob = NULL; // output

		hr = D3DCompileFromFile(L"shader/DefaultShader.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainvs", "vs_5_0", dwShaderFlags, 0, &vsBlob, &pBlobError);
		if (FAILED(hr))
		{
			if (pBlobError != NULL)
			{
				OutputDebugStringA((CHAR*)pBlobError->GetBufferPointer());
				pBlobError->Release();
			}
			Assert(1 == 2);
		}

		ID3D11VertexShader* MainVertexShader = NULL;
		hr = Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &MainVertexShader);
		if (FAILED(hr))
		{
			vsBlob->Release();
			Assert(1 == 2);
		}

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",
				0, DXGI_FORMAT_R32G32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",
				0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",
				0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BITANGENT",
				0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA, 0 },

			{ "SV_InstanceID", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		};

		hr = Device->CreateInputLayout(layout,
			ArrayCount(layout),
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			&DefaultVertexLayout);
		if (FAILED(hr))
		{
			vsBlob->Release();
			Assert(1 == 2);
		}

		DeviceContext->IASetInputLayout(DefaultVertexLayout);

		vsBlob->Release();

		shader Shader = shader();
		Shader.Name = "mainvs";
		Shader.ShaderType = shader_type::VertexShader;
		Shader.ShaderRef = MainVertexShader;
		Engine->ShaderRegistry.push_back(Shader);
	}

	//
	// Create main vertex buffer
	//
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(vertex) * 200000; // MAX BUFFER SIZE
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = Device->CreateBuffer(&bufferDesc, NULL, &MainVertexBuffer);
	if (FAILED(hr))
		Assert(1 == 2);;

	bufferDesc.ByteWidth = sizeof(vertex) * 4000;

	hr = Device->CreateBuffer(&bufferDesc, NULL, &TerrainChunkVertexBuffer);
	if (FAILED(hr))
		Assert(1 == 2);;


	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(u32) * 200000; // MAX BUFFER SIZE

	hr = Device->CreateBuffer(&bufferDesc, NULL, &MainIndexBuffer);
	if (FAILED(hr))
		Assert(1 == 2);;

	bufferDesc.ByteWidth = sizeof(u32) * 6000;

	hr = Device->CreateBuffer(&bufferDesc, NULL, &TerrainChunkIndexBuffer);
	if (FAILED(hr))
		Assert(1 == 2);;

	// map terrain index buffer because it will always stay the same
	//D3D11_MAPPED_SUBRESOURCE Mapped;
	//u32 indexes[6] = { 0, 1, 2, 0, 3, 1 };
	//DeviceContext->Map(TerrainChunkIndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
	//memcpy(Mapped.pData, indexes, sizeof(u32) * 6);
	//DeviceContext->Unmap(TerrainChunkIndexBuffer, NULL);

	// Create frame constant buffer
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = sizeof(shader_constants_frame); // CONST BUFFER SIZE
	hr = Device->CreateBuffer(&bufferDesc, NULL, &FrameConstantBuffer);
	if (FAILED(hr))
		Assert(1 == 2);
	DeviceContext->VSSetConstantBuffers(0, 1, &FrameConstantBuffer);
	DeviceContext->DSSetConstantBuffers(0, 1, &FrameConstantBuffer);
	DeviceContext->HSSetConstantBuffers(0, 1, &FrameConstantBuffer);
	//DeviceContext->PSSetConstantBuffers(0, 1, &ActorConstantBuffer);
	//DeviceContext->GSSetConstantBuffers(0, 1, &ActorConstantBuffer);

	// Create actor constant buffer
	bufferDesc.ByteWidth = sizeof(shader_constants_actor) * MAX_INSTANCES; // MAX instances
	hr = Device->CreateBuffer(&bufferDesc, NULL, &ActorConstantBuffer);
	if (FAILED(hr))
		Assert(1 == 2);;
	DeviceContext->VSSetConstantBuffers(1, 1, &ActorConstantBuffer);
	DeviceContext->DSSetConstantBuffers(1, 1, &ActorConstantBuffer);

	// Create material constant buffer
	bufferDesc.ByteWidth = sizeof(shader_constants_material); // CONST BUFFER SIZE
	hr = Device->CreateBuffer(&bufferDesc, NULL, &MaterialConstantBuffer);
	if (FAILED(hr))
		Assert(1 == 2);
	DeviceContext->PSSetConstantBuffers(0, 1, &MaterialConstantBuffer);

	// Create lighting constant buffer
	bufferDesc.ByteWidth = sizeof(shader_constants_lighting); // CONST BUFFER SIZE
	hr = Device->CreateBuffer(&bufferDesc, NULL, &LightingConstantBuffer);
	if (FAILED(hr))
		Assert(1 == 2);
	DeviceContext->PSSetConstantBuffers(1, 1, &LightingConstantBuffer);

	//
	// create texture sampler
	// NOTE: If you do not set a samplerstate, the default will be MIN_MAG_MIP_LINEAR
	//
	D3D11_SAMPLER_DESC SamplerDesc;
	SamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	//SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	//SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.MipLODBias = 0;
	SamplerDesc.MaxAnisotropy = 4;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerDesc.BorderColor[0] = 1.0f;
	SamplerDesc.BorderColor[1] = 1.0f;
	SamplerDesc.BorderColor[2] = 1.0f;
	SamplerDesc.BorderColor[3] = 1.0f;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Device->CreateSamplerState(&SamplerDesc, &DefaultSampler);
	DeviceContext->PSSetSamplers(0, 1, &DefaultSampler);
	DeviceContext->DSSetSamplers(0, 1, &DefaultSampler);
	//DeviceContext->VSSetSamplers(0, 1, &DefaultSampler);

	//
	// Create blend state
	//
	D3D11_BLEND_DESC BlendDesc = {};

	BlendDesc.RenderTarget[0].BlendEnable = true;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = Device->CreateBlendState(&BlendDesc, &BlendState);
	DeviceContext->OMSetBlendState(BlendState, NULL, 0xffffffff);

	//
	// Setup depth stencil texture
	//
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = WindowWidth;
	descDepth.Height = WindowHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = MultiSamplingCount;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	hr = Device->CreateTexture2D(&descDepth, NULL, &DepthStencilTex);
	if (FAILED(hr))
		Assert(1 == 2);

	//
	// Setup depth view
	//
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));

	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	descDSV.Texture2D.MipSlice = 0;

	hr = Device->CreateDepthStencilView(DepthStencilTex, &descDSV, &DepthStencilView);
	if (FAILED(hr))
		Assert(1 == 2);

	DeviceContext->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);

	//
	// Create two depth stencil states
	//
	D3D11_DEPTH_STENCIL_DESC DepthDesc = {};
	DepthDesc.DepthEnable = true;
	DepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	DepthDesc.StencilEnable = false;
	DepthDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	DepthDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	DepthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	DepthDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	DepthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	DepthDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	DepthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DepthDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DepthDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DepthDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	hr = Device->CreateDepthStencilState(&DepthDesc, &DepthStencilEnabled);
	if (FAILED(hr))
		Assert(1 == 2);

	DepthDesc.DepthEnable = false;

	hr = Device->CreateDepthStencilState(&DepthDesc, &DepthStencilDisabled);
	if (FAILED(hr))
		Assert(1 == 2);

	DeviceContext->OMSetDepthStencilState(DepthStencilEnabled, 1);

	// TODO replace with engine::UpdateResolution()
	GetClientRect(Window, &rc);
	SetViewport((FLOAT)(rc.right - rc.left), (FLOAT)(rc.bottom - rc.top));

	// lock cursor to window
	POINT ul;
	ul.x = rc.left;
	ul.y = rc.top;

	POINT lr;
	lr.x = rc.right;
	lr.y = rc.bottom;

	MapWindowPoints(Window, nullptr, &ul, 1);
	MapWindowPoints(Window, nullptr, &lr, 1);

	rc.left = ul.x;
	rc.top = ul.y;

	rc.right = lr.x;
	rc.bottom = lr.y;

	ClipCursor(&rc);

	// init imgui
	ImGui_ImplWin32_Init(Window);
	ImGui_ImplDX11_Init(Device, DeviceContext);
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
}

void dx11_renderer::Cleanup()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	for (u32 i = 0; i < Engine->AssetRegistry.size(); i++)
	{
		Engine->AssetRegistry[i]->UnloadAsset();
	}

	for (u32 i = 0; i < Engine->ShaderRegistry.size(); i++)
	{
		switch (Engine->ShaderRegistry[i].ShaderType)
		{
			case shader_type::VertexShader:
			{ SAFE_RELEASE(((ID3D11VertexShader*)Engine->ShaderRegistry[i].ShaderRef)); } break;

			case shader_type::PixelShader:
			{ SAFE_RELEASE(((ID3D11PixelShader*)Engine->ShaderRegistry[i].ShaderRef)); } break;

			case shader_type::GeometryShader:
			{ SAFE_RELEASE(((ID3D11GeometryShader*)Engine->ShaderRegistry[i].ShaderRef)); } break;

			case shader_type::HullShader:
			{ SAFE_RELEASE(((ID3D11HullShader*)Engine->ShaderRegistry[i].ShaderRef)); } break;

			case shader_type::DomainShader:
			{ SAFE_RELEASE(((ID3D11DomainShader*)Engine->ShaderRegistry[i].ShaderRef)); } break;
		}
	}

	SAFE_RELEASE(MainIndexBuffer);
	SAFE_RELEASE(TerrainChunkIndexBuffer);
	SAFE_RELEASE(MainVertexBuffer);
	SAFE_RELEASE(TerrainChunkVertexBuffer);
	SAFE_RELEASE(ActorConstantBuffer);
	SAFE_RELEASE(MaterialConstantBuffer);
	SAFE_RELEASE(FrameConstantBuffer);
	SAFE_RELEASE(LightingConstantBuffer);

	SAFE_RELEASE(SkyboxCube);

	//layouts
	SAFE_RELEASE(DefaultVertexLayout);

	SAFE_RELEASE(BlendState);
	SAFE_RELEASE(DefaultCullBackface);
	SAFE_RELEASE(DefaultCullFrontface);
	SAFE_RELEASE(DefaultCullNone);
	SAFE_RELEASE(Wireframe);
	SAFE_RELEASE(DepthStencilEnabled);
	SAFE_RELEASE(DepthStencilDisabled);
	SAFE_RELEASE(DepthStencilTex);
	SAFE_RELEASE(DepthStencilView);
	SAFE_RELEASE(RenderTargetView);
	SAFE_RELEASE(DefaultSampler);
	SAFE_RELEASE(Chain);

#if SPADE_DEBUG
	//DebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	SAFE_RELEASE(DebugDevice);
#endif

	SAFE_RELEASE(DeviceContext);
	SAFE_RELEASE(Device);
}

void dx11_renderer::FinishFrame()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	Chain->Present(0, 0);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	DeviceContext->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
	DeviceContext->ClearRenderTargetView(RenderTargetView, VoidColor);
	DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

/*
* Sets topology type
*/
void dx11_renderer::Draw(vertex* InVertexArray, u32 NumVertices, draw_topology_type TopologyType)
{
	D3D11_MAPPED_SUBRESOURCE Mapped;
	DeviceContext->Map(MainVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
	memcpy(Mapped.pData, InVertexArray, sizeof(vertex) * NumVertices);
	DeviceContext->Unmap(MainVertexBuffer, NULL);

	UINT stride = sizeof(vertex);
	UINT offset = 0;

	SetDrawTopology(TopologyType);

	DeviceContext->IASetVertexBuffers(0, 1, &MainVertexBuffer, &stride, &offset);
	DeviceContext->Draw(NumVertices, 0);
}

void dx11_renderer::Draw(v3* InPositionArray, u32 NumVertices, draw_topology_type TopologyType)
{
	D3D11_MAPPED_SUBRESOURCE Mapped;
	DeviceContext->Map(PositionVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
	memcpy(Mapped.pData, InPositionArray, sizeof(v3) * NumVertices);
	DeviceContext->Unmap(PositionVertexBuffer, NULL);

	UINT stride = sizeof(v3);
	UINT offset = 0;

	SetDrawTopology(TopologyType);

	DeviceContext->IASetVertexBuffers(0, 1, &PositionVertexBuffer, &stride, &offset);
	DeviceContext->Draw(NumVertices, 0);
}

void dx11_renderer::DrawInstanced(vertex* InVertexArray, u32 NumVertices, u32 NumInstances, draw_topology_type TopologyType)
{
	D3D11_MAPPED_SUBRESOURCE Mapped;
	DeviceContext->Map(MainVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
	memcpy(Mapped.pData, InVertexArray, sizeof(vertex) * NumVertices);
	DeviceContext->Unmap(MainVertexBuffer, NULL);

	UINT stride = sizeof(vertex);
	UINT offset = 0;

	SetDrawTopology(TopologyType);

	DeviceContext->IASetVertexBuffers(0, 1, &MainVertexBuffer, &stride, &offset);
	DeviceContext->DrawInstanced(NumVertices, NumInstances, 0, 0);
}

void dx11_renderer::DrawIndexedInstanced(vertex* InVertexArray, u32* InIndexArray, u32 NumVertices, u32 NumIndices, u32 IndexOffset, u32 NumInstances, draw_topology_type TopologyType)
{
	D3D11_MAPPED_SUBRESOURCE Mapped;
	DeviceContext->Map(MainVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
	memcpy(Mapped.pData, InVertexArray, sizeof(vertex) * NumVertices);
	DeviceContext->Unmap(MainVertexBuffer, NULL);

	DeviceContext->Map(MainIndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
	memcpy(Mapped.pData, InIndexArray, sizeof(u32) * NumIndices);
	DeviceContext->Unmap(MainIndexBuffer, NULL);

	UINT stride = sizeof(vertex);
	UINT offset = 0;

	SetDrawTopology(TopologyType);

	DeviceContext->IASetVertexBuffers(0, 1, &MainVertexBuffer, &stride, &offset);
	DeviceContext->IASetIndexBuffer(MainIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->DrawIndexedInstanced(NumIndices, NumInstances, IndexOffset, 0, 0);
}

void dx11_renderer::DrawIndexedTerrainChunk(vertex* InVertexArray, u32* InIndexArray, u32 NumVertices, u32 NumIndices)
{
	D3D11_MAPPED_SUBRESOURCE Mapped;
	DeviceContext->Map(TerrainChunkVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
	memcpy(Mapped.pData, InVertexArray, sizeof(vertex) * NumVertices);
	DeviceContext->Unmap(TerrainChunkVertexBuffer, NULL);

	DeviceContext->Map(TerrainChunkIndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
	memcpy(Mapped.pData, InIndexArray, sizeof(u32) * NumIndices);
	DeviceContext->Unmap(TerrainChunkIndexBuffer, NULL);

	UINT stride = sizeof(vertex);
	UINT offset = 0;

	SetDrawTopology(draw_topology_type::TriangleList);

	DeviceContext->IASetVertexBuffers(0, 1, &TerrainChunkVertexBuffer, &stride, &offset);
	DeviceContext->IASetIndexBuffer(TerrainChunkIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->DrawIndexedInstanced(NumIndices, 1, 0, 0, 0);
}

void dx11_renderer::SetViewport(float Width, float Height)
{
	D3D11_VIEWPORT vp;
	ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));
	vp.Width = Width;
	vp.Height = Height;
	vp.TopLeftX = 0.f;
	vp.TopLeftY = 0.f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	DeviceContext->RSSetViewports(1, &vp);
}

void dx11_renderer::SetDrawTopology(draw_topology_type TopologyType)
{
	if (CurrentState.EnableTesselation) // override top type if tesselation is enabled (assumes triangulated vertex inputs)
		DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	else
	{
		switch (TopologyType)
		{
		default:
		case draw_topology_type::TriangleList:
		{ DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); } break;

		case draw_topology_type::LineList:
		{ DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST); } break;

		case draw_topology_type::PointList:
		{ DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST); } break;
		}
	}
}

void dx11_renderer::CompileShaderFromFile(std::string Filename, std::string EntryPoint, shader_type ShaderType, void* ShaderRef)
{
	HRESULT hr;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

	ID3DBlob* pBlobError = NULL;
	ID3DBlob* vsBlob = NULL; // output

	std::string Target;
	switch (ShaderType)
	{
		case shader_type::VertexShader:
		{
			Target = "vs_5_0";
		} break;
		case shader_type::PixelShader:
		{
			Target = "ps_5_0";
		} break;
		case shader_type::GeometryShader:
		{
			Target = "gs_5_0";
		} break;
		case shader_type::HullShader:
		{
			Target = "hs_5_0";
		} break;
		case shader_type::DomainShader:
		{
			Target = "ds_5_0";
		} break;
	}

	// not compatible w unicode
	std::wstring fn(Filename.begin(), Filename.end());
	hr = D3DCompileFromFile(fn.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), Target.c_str(), dwShaderFlags, 0, &vsBlob, &pBlobError);
	if (FAILED(hr))
	{
		if (pBlobError != NULL)
		{
			OutputDebugStringA((CHAR*)pBlobError->GetBufferPointer());
			pBlobError->Release();
		}
		Assert(1 == 2);
	}

	switch (ShaderType)
	{
		case shader_type::VertexShader:
		{
			hr = Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, (ID3D11VertexShader**)ShaderRef);
		} break;
		case shader_type::PixelShader:
		{
			hr = Device->CreatePixelShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, (ID3D11PixelShader**)ShaderRef);
		} break;
		case shader_type::GeometryShader:
		{
			hr = Device->CreateGeometryShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, (ID3D11GeometryShader**)ShaderRef);
		} break;
		case shader_type::HullShader:
		{
			hr = Device->CreateHullShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, (ID3D11HullShader**)ShaderRef);
		} break;
		case shader_type::DomainShader:
		{
			hr = Device->CreateDomainShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, (ID3D11DomainShader**)ShaderRef);
		} break;
	}

	if (FAILED(hr))
	{
		vsBlob->Release();
		Assert(1 == 2);
	}

	vsBlob->Release();
}

void dx11_renderer::RegisterTexture(cAsset* Asset, bool GenerateMIPs)
{
	assetLoader::RegisterDXTexture(Asset, GenerateMIPs, Device, DeviceContext);
}

void dx11_renderer::BindMaterial(const material& InMaterial)
{
	DeviceContext->PSSetShaderResources(0, 1, &SkyboxCube);
	if (CurrentState.UniqueIdentifier == "DefaultPBR")
	{
		MaterialConstants.TextureDiffuse = InMaterial.DiffuseTextureID == -1 ? false : true;
		MaterialConstants.TextureNormal = InMaterial.NormalTextureID == -1 ? false : true;
		MaterialConstants.TextureReflective = InMaterial.ReflectiveTextureID == -1 ? false : true;
		MaterialConstants.DiffuseColor = InMaterial.DiffuseColor;
		MaterialConstants.Reflectivity = InMaterial.Reflectivity;

		MapBuffer(MaterialConstantBuffer, MaterialConstants);

		ID3D11ShaderResourceView* const Views[2] = { MaterialConstants.TextureDiffuse ? Engine->TextureRegistry[InMaterial.DiffuseTextureID]->ShaderHandle : NULL,
													 MaterialConstants.TextureNormal ? Engine->TextureRegistry[InMaterial.NormalTextureID]->ShaderHandle : NULL };

		DeviceContext->PSSetShaderResources(1, 2, Views);
		DeviceContext->DSSetShaderResources(1, 2, Views);
	}
	//else if (CurrentState.UniqueIdentifier == "Skybox")
	//{
	//	
	//}
}

void dx11_renderer::MapConstants(map_operation Type)
{
	switch (Type)
	{
		case map_operation::Actor:
		{ MapBuffer(ActorConstantBuffer, ActorConstants); } break;

		case map_operation::Frame:
		{ MapBuffer(FrameConstantBuffer, FrameConstants); } break;

		case map_operation::Material:
		{ MapBuffer(MaterialConstantBuffer, MaterialConstants); } break;

		case map_operation::Lighting:
		{ MapBuffer(LightingConstantBuffer, LightingConstants); } break;
	}
}

void dx11_renderer::UpdateSkybox(s32* TextureIDs)
{
	SAFE_RELEASE(SkyboxCube);
	s32 Width = Engine->TextureRegistry[TextureIDs[0]]->ImageData.Width;
	s32 Channels = Engine->TextureRegistry[TextureIDs[0]]->ImageData.Channels;

	u32 MipLevels = 10;

	D3D11_TEXTURE2D_DESC descDepth1;
	ZeroMemory(&descDepth1, sizeof(descDepth1));
	descDepth1.Width = Width;
	descDepth1.Height = Width;
	descDepth1.MipLevels = MipLevels;
	descDepth1.ArraySize = 6;
	descDepth1.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	descDepth1.SampleDesc.Count = 1;
	descDepth1.SampleDesc.Quality = 0;
	descDepth1.Usage = D3D11_USAGE_DEFAULT;
	descDepth1.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	descDepth1.CPUAccessFlags = 0;
	descDepth1.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ID3D11Texture2D* tex;
	HRESULT hr = Device->CreateTexture2D(&descDepth1, NULL, &tex);
	if (FAILED(hr))
		Assert(1 == 2);

	for (u32 i = 0; i < 6; i++)
	{
		DeviceContext->UpdateSubresource(tex, D3D11CalcSubresource(0, i, MipLevels), NULL, Engine->TextureRegistry[TextureIDs[i]]->Data, Width * Channels, 0);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2DArray.MipLevels = MipLevels;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = 6;

	hr = Device->CreateShaderResourceView((ID3D11Resource*)tex, &srvDesc, &SkyboxCube);
	if (FAILED(hr))
		Assert(1 == 2);

	DeviceContext->GenerateMips(SkyboxCube);

	SAFE_RELEASE(tex);
}

void dx11_renderer::SetPipelineState(const pipeline_state& InState)
{
	// vertex shader
	if (CurrentState.VertexShaderID != InState.VertexShaderID)
	{
		if (InState.VertexShaderID == -1)
			DeviceContext->VSSetShader(NULL, NULL, 0);
		else
		{
			Assert(Engine->ShaderRegistry[InState.VertexShaderID].ShaderType == shader_type::VertexShader)
				DeviceContext->VSSetShader((ID3D11VertexShader*)Engine->ShaderRegistry[InState.VertexShaderID].ShaderRef, NULL, 0);
		}
	}

	// pixel shader
	if (CurrentState.PixelShaderID != InState.PixelShaderID)
	{
		if (InState.PixelShaderID == -1)
			DeviceContext->PSSetShader(NULL, NULL, 0);
		else
		{
			Assert(Engine->ShaderRegistry[InState.PixelShaderID].ShaderType == shader_type::PixelShader)
				DeviceContext->PSSetShader((ID3D11PixelShader*)Engine->ShaderRegistry[InState.PixelShaderID].ShaderRef, NULL, 0);
		}
	}

	// geo shader
	if (CurrentState.GeometryShaderID != InState.GeometryShaderID)
	{
		if (InState.GeometryShaderID == -1)
			DeviceContext->GSSetShader(NULL, NULL, 0);
		else
		{
			Assert(Engine->ShaderRegistry[InState.GeometryShaderID].ShaderType == shader_type::GeometryShader)
				DeviceContext->GSSetShader((ID3D11GeometryShader*)Engine->ShaderRegistry[InState.GeometryShaderID].ShaderRef, NULL, 0);
		}
	}

	if (CurrentState.EnableTesselation != InState.EnableTesselation)
	{
		if (InState.HullShaderID == -1 || InState.EnableTesselation == false)
			DeviceContext->HSSetShader(NULL, NULL, 0);
		else
		{
			Assert(Engine->ShaderRegistry[InState.HullShaderID].ShaderType == shader_type::HullShader)
				DeviceContext->HSSetShader((ID3D11HullShader*)Engine->ShaderRegistry[InState.HullShaderID].ShaderRef, NULL, 0);
		}

		if (InState.DomainShaderID == -1 || InState.EnableTesselation == false)
			DeviceContext->DSSetShader(NULL, NULL, 0);
		else
		{
			Assert(Engine->ShaderRegistry[InState.DomainShaderID].ShaderType == shader_type::DomainShader)
				DeviceContext->DSSetShader((ID3D11DomainShader*)Engine->ShaderRegistry[InState.DomainShaderID].ShaderRef, NULL, 0);
		}
	}

	// rasterizer state
	if (CurrentState.RasterizerState != InState.RasterizerState)
	{
		switch (InState.RasterizerState)
		{
			case rasterizer_state::DefaultCullBackface:
			{ DeviceContext->RSSetState(DefaultCullBackface); } break;

			case rasterizer_state::DefaultCullFrontface:
			{ DeviceContext->RSSetState(DefaultCullFrontface); } break;

			case rasterizer_state::DefaultCullNone:
			{ DeviceContext->RSSetState(DefaultCullNone); } break;

			case rasterizer_state::Wireframe:
			{ DeviceContext->RSSetState(Wireframe); } break;
		}
	}

	CurrentState = InState;
}

matrix4x4 dx11_renderer::GetPerspectiveProjectionLH(bool Transpose, camera_info CameraInfo)
{
	matrix4x4 Result;

	DirectX::XMMATRIX Projection = DirectX::XMMatrixPerspectiveFovLH(CameraInfo.FOV * (Pi32 / 180.0f), CameraInfo.Width / CameraInfo.Height, CameraInfo.NearPlane, CameraInfo.FarPlane);
	if (Transpose)
	{
		Projection = DirectX::XMMatrixTranspose(Projection);
	}

	DirectX::XMFLOAT4X4 Temp;
	DirectX::XMStoreFloat4x4(&Temp, Projection);

	Result.row1 = v4{ Temp._11, Temp._12, Temp._13, Temp._14 };
	Result.row2 = v4{ Temp._21, Temp._22, Temp._23, Temp._24 };
	Result.row3 = v4{ Temp._31, Temp._32, Temp._33, Temp._34 };
	Result.row4 = v4{ Temp._41, Temp._42, Temp._43, Temp._44 };

	return Result;
}

matrix4x4 dx11_renderer::GetOrthographicProjectionLH(bool Transpose, camera_info CameraInfo)
{
	matrix4x4 Result;

	DirectX::XMMATRIX Projection = DirectX::XMMatrixOrthographicLH(CameraInfo.Width, CameraInfo.Height, CameraInfo.NearPlane, CameraInfo.FarPlane);
	if (Transpose)
	{
		Projection = DirectX::XMMatrixTranspose(Projection);
	}

	DirectX::XMFLOAT4X4 Temp;
	DirectX::XMStoreFloat4x4(&Temp, Projection);

	Result.row1 = v4{ Temp._11, Temp._12, Temp._13, Temp._14 };
	Result.row2 = v4{ Temp._21, Temp._22, Temp._23, Temp._24 };
	Result.row3 = v4{ Temp._31, Temp._32, Temp._33, Temp._34 };
	Result.row4 = v4{ Temp._41, Temp._42, Temp._43, Temp._44 };

	return Result;
}

matrix4x4 dx11_renderer::GenerateViewMatrix(bool Transpose, camera_info CameraInfo, v3& OutLookAtVector, v3& OutUpVector, bool OrthoUseMovement)
{
	DirectX::XMVECTOR defaultForward;
	DirectX::XMVECTOR camUp;
	DirectX::XMVECTOR camRight;
	DirectX::XMVECTOR camPosition;
	DirectX::XMVECTOR camTarget;
	DirectX::XMMATRIX CameraView;

	if (CameraInfo.ProjectionType == projection_type::Perspective)
	{
		f32 DebugCameraOffset = -0.0f;

		defaultForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		camUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		camRight = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		//camTarget = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		camPosition = DirectX::XMVectorSet(CameraInfo.Transform.Location.x, CameraInfo.Transform.Location.y, CameraInfo.Transform.Location.z, 0.0f);

		// Rotation Matrix

		DirectX::XMVECTOR Quat = DirectX::XMQuaternionRotationAxis(camRight, DegreesToRadians(CameraInfo.Transform.Rotation.y));
		Quat = DirectX::XMQuaternionMultiply(Quat, DirectX::XMQuaternionRotationAxis(camUp, DegreesToRadians(CameraInfo.Transform.Rotation.x)));
		DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationQuaternion(Quat);// DirectX::XMMatrixRotationQuaternion(Quat);
		//DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(CameraInfo.Transform.Rotation.y), DirectX::XMConvertToRadians(CameraInfo.Transform.Rotation.x), 0.0f);
		DirectX::XMVECTOR camTarget = DirectX::XMVector3TransformCoord(defaultForward, RotationMatrix);
		//camTarget = DirectX::XMVector3Normalize(camTarget);

		DirectX::XMFLOAT4 temp;    //the float where we copy the v2 vector members
		DirectX::XMStoreFloat4(&temp, camTarget);   //the function used to copy
		OutLookAtVector.x = temp.x;
		OutLookAtVector.y = temp.y;
		OutLookAtVector.z = temp.z;

		camUp = XMVector3TransformCoord(camUp, RotationMatrix);
		DirectX::XMStoreFloat4(&temp, camUp);   //the function used to copy
		OutUpVector.x = temp.x;
		OutUpVector.y = temp.y;
		OutUpVector.z = temp.z;

		camTarget = DirectX::XMVectorAdd(camTarget, camPosition);

		if (Transpose)
			CameraView = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(camPosition, camTarget, camUp));
		else
			CameraView = DirectX::XMMatrixLookAtLH(camPosition, camTarget, camUp);

		//CameraView = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixMultiply(RotationMatrix, DirectX::XMMatrixTranslationFromVector(camPosition)));
	}
	else
	{
		camUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		if (OrthoUseMovement)
			camPosition = DirectX::XMVectorSet(CameraInfo.Transform.Location.x, CameraInfo.Transform.Location.y, CameraInfo.Transform.Location.z, 0.0f);
		else
			camPosition = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		camTarget = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		CameraView = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(camPosition, camTarget, camUp));
	}

	DirectX::XMFLOAT4X4 View;
	DirectX::XMStoreFloat4x4(&View, CameraView);

	matrix4x4 Result;
	Result.m11 = View._11;
	Result.m12 = View._12;
	Result.m13 = View._13;
	Result.m14 = View._14;
	Result.m21 = View._21;
	Result.m22 = View._22;
	Result.m23 = View._23;
	Result.m24 = View._24;
	Result.m31 = View._31;
	Result.m32 = View._32;
	Result.m33 = View._33;
	Result.m34 = View._34;
	Result.m41 = View._41;
	Result.m42 = View._42;
	Result.m43 = View._43;
	Result.m44 = View._44;

	return Result;
}

matrix4x4 dx11_renderer::GenerateWorldMatrix(transform Transform)
{
	DirectX::XMMATRIX translation, rotationx, rotationy, rotationz, scale;
	translation = DirectX::XMMatrixTranslation(Transform.Location.x, Transform.Location.y, Transform.Location.z);
	rotationx = DirectX::XMMatrixRotationX(DegreesToRadians(Transform.Rotation.x)); // convert degrees to radians
	rotationy = DirectX::XMMatrixRotationY(DegreesToRadians(Transform.Rotation.y));
	rotationz = DirectX::XMMatrixRotationZ(DegreesToRadians(Transform.Rotation.z));
	scale = DirectX::XMMatrixScaling(Transform.Scale.x, Transform.Scale.y, Transform.Scale.z);

	// transform order: scale, rotate (degrees), translate
	return ToMatrix4x4(scale * rotationx * rotationy * rotationz * translation);
}

matrix4x4 dx11_renderer::InverseMatrix(const matrix4x4& Matrix, bool Transpose)
{
	DirectX::XMMATRIX Inverse = ToDXM(Matrix);
	Inverse = DirectX::XMMatrixInverse(nullptr, Inverse);
	if (true)
		Inverse = DirectX::XMMatrixTranspose(Inverse);
	return ToMatrix4x4(Inverse);
}

template<typename AssociatedStruct>
void dx11_renderer::MapBuffer(void* BufferRef, AssociatedStruct Data)
{
	D3D11_MAPPED_SUBRESOURCE Mapped;
	DeviceContext->Map((ID3D11Buffer*)BufferRef, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
	AssociatedStruct* dataPtr = (AssociatedStruct*)Mapped.pData;
	*dataPtr = Data;
	DeviceContext->Unmap((ID3D11Buffer*)BufferRef, NULL);
}