#include "DX11Renderer.h"

void dx11_renderer::Initialize(HWND Window, int WindowWidth, int WindowHeight)
{
	//
	// Device and swap chain
	//
	UINT MultiSamplingCount = 4;
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = WindowWidth;
	sd.BufferDesc.Height = WindowHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = Window;
	sd.SampleDesc.Count = MultiSamplingCount;
	sd.SampleDesc.Quality = 0;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Windowed = TRUE;

	const D3D_FEATURE_LEVEL lvl[] = {
		//D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	D3D_FEATURE_LEVEL FeatureLevelsSupported;
	UINT createDeviceFlags = 0;
#if SPADE_DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, lvl, _countof(lvl), D3D11_SDK_VERSION, &sd, &Chain, &Device, &FeatureLevelsSupported, &DeviceContext);
	if (hr == E_INVALIDARG)
	{
		hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, &lvl[1], _countof(lvl) - 1, D3D11_SDK_VERSION, &sd, &Chain, &Device, &FeatureLevelsSupported, &DeviceContext);
	}
	if (FAILED(hr))
		Assert(1 == 2);

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
	ID3D11RasterizerState* m_rasterState;
	D3D11_RASTERIZER_DESC rasterDesc;
	// Setup a raster description which turns off back face culling.
	rasterDesc.AntialiasedLineEnable = true;
	//rasterDesc.CullMode = D3D11_CULL_NONE;
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
	hr = Device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(hr))
		Assert(1 == 2);

	// Now set the rasterizer state.
	DeviceContext->RSSetState(m_rasterState);
	m_rasterState->Release();


	//
	// Compile default vs, gs, and ps shaders
	//
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
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
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
		DeviceContext->VSSetShader(MainVertexShader, NULL, 0);

		vsBlob->Release();
	}

	// PIXEL SHADER
	CompileShaderFromFile(L"shader/DefaultShader.hlsl", "mainps", shader_type::PixelShader, &MainPixelShader);
	DeviceContext->PSSetShader(MainPixelShader, NULL, 0);

	// GEO SHADER
	//CompileShaderFromFile(L"shader/DefaultShader.hlsl", "maings", shader_type::GeometryShader, &MainGeoShader);
	//CompileShaderFromFile(L"shader/DefaultShader.hlsl", "geovs", shader_type::VertexShader, &MainGeoVertexShader);

	//// circle pixel shader
	//CompileShaderFromFile(L"shader/DefaultShader.hlsl", "circleps", shader_type::PixelShader, &CirclePixelShader);

	//// compile tile shaders
	//CompileShaderFromFile(L"shader/TileShader.hlsl", "quadvs", shader_type::VertexShader, &TileVertexShader);
	//CompileShaderFromFile(L"shader/TileShader.hlsl", "quadps", shader_type::PixelShader, &TilePixelShader);
	//CompileShaderFromFile(L"shader/TileShader.hlsl", "quadgs", shader_type::GeometryShader, &TileGeoShader);

	//// compile 2D font rendering shaders
	//CompileShaderFromFile(L"shader/2dfontshader.hlsl", "mainvs", shader_type::VertexShader, &TwoDFontVertShader);
	//CompileShaderFromFile(L"shader/2dfontshader.hlsl", "mainps", shader_type::PixelShader, &TwoDFontPixShader);


	//
	// Create main vertex buffer
	//
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(vertex) * 100000; // MAX BUFFER SIZE
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = Device->CreateBuffer(&bufferDesc, NULL, &MainVertexBuffer);
	if (FAILED(hr))
		Assert(1 == 2);;


	//
	// Create main constant buffer
	//
	//ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	//bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	//bufferDesc.ByteWidth = sizeof(shader_constants); // CONST BUFFER SIZE
	//bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//hr = Device->CreateBuffer(&bufferDesc, NULL, &ConstantBuffer);
	//if (FAILED(hr))
	//	Assert(1 == 2);

	//DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	//DeviceContext->PSSetConstantBuffers(0, 1, &ConstantBuffer);
	//DeviceContext->GSSetConstantBuffers(0, 1, &ConstantBuffer);

	//
	// Create lighting constant buffer
	//
	//ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	//bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	//bufferDesc.ByteWidth = sizeof(light_shader_constants); // CONST BUFFER SIZE
	//bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//hr = Device->CreateBuffer(&bufferDesc, NULL, &LightingBuffer);
	//if (FAILED(hr))
	//	Assert(1 == 2);

	//DeviceContext->PSSetConstantBuffers(1, 1, &LightingBuffer);

	//
	// create texture sampler
	// NOTE: If you do not set a samplerstate, the default will be MIN_MAG_MIP_LINEAR
	//
	D3D11_SAMPLER_DESC SamplerDesc;
	//SamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.MipLODBias = 0;
	SamplerDesc.MaxAnisotropy = 2;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerDesc.BorderColor[0] = 1.0f;
	SamplerDesc.BorderColor[1] = 1.0f;
	SamplerDesc.BorderColor[2] = 1.0f;
	SamplerDesc.BorderColor[3] = 1.0f;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Device->CreateSamplerState(&SamplerDesc, &DefaultSampler);
	DeviceContext->PSSetSamplers(0, 1, &DefaultSampler);


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
	DeviceContext->OMSetBlendState(BlendState, 0, 0xffffffff);

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
}

void dx11_renderer::Cleanup()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//for (u32 i = 0; i < GlobalTextureRegistry.Num(); i++)
	//{
	//	SAFE_RELEASE(GlobalTextureRegistry[i]->TextureHandle);
	//	SAFE_RELEASE(GlobalTextureRegistry[i]->ShaderHandle);
	//}
	//for (u32 i = 0; i < GlobalAssetRegistry.Num(); i++)
	//{
	//	if (GlobalAssetRegistry[i]->Type == asset_type::Font)
	//	{
	//		SAFE_RELEASE(((cFontAsset*)GlobalAssetRegistry[i])->AtlasShaderHandle);
	//	}
	//}

	//remove
	//SAFE_RELEASE(MainBuffer);
	SAFE_RELEASE(ConstantBuffer);
	//SAFE_RELEASE(LightingBuffer);

	//layouts
	SAFE_RELEASE(DefaultVertexLayout);

	//vertex shaders
	SAFE_RELEASE(MainVertexShader);
	//SAFE_RELEASE(TileVertexShader);
	//SAFE_RELEASE(MainGeoVertexShader);
	//SAFE_RELEASE(TwoDFontVertShader);

	////pixel shaders
	//SAFE_RELEASE(TilePixelShader);
	SAFE_RELEASE(MainPixelShader);
	//SAFE_RELEASE(CirclePixelShader);
	//SAFE_RELEASE(TwoDFontPixShader);

	////geo shaders
	//SAFE_RELEASE(MainGeoShader);
	//SAFE_RELEASE(TileGeoShader);

	SAFE_RELEASE(BlendState);
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

/*
* Sets topology type
*/
void dx11_renderer::Draw(std::vector<vertex>& VertexArray, draw_topology_types TopologyType)
{
	D3D11_MAPPED_SUBRESOURCE Mapped;
	DeviceContext->Map(MainVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
	memcpy(Mapped.pData, VertexArray.data(), sizeof(vertex) * VertexArray.size());
	DeviceContext->Unmap(MainVertexBuffer, NULL);

	UINT stride = sizeof(vertex);
	UINT offset = 0;

	SetDrawTopology(TopologyType);

	DeviceContext->IASetVertexBuffers(0, 1, &MainVertexBuffer, &stride, &offset);
	DeviceContext->Draw((u32)VertexArray.size(), 0);
}

void dx11_renderer::Draw(std::vector<v3>& PositionArray, draw_topology_types TopologyType)
{
	D3D11_MAPPED_SUBRESOURCE Mapped;
	DeviceContext->Map(PositionVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
	memcpy(Mapped.pData, PositionArray.data(), sizeof(v3) * PositionArray.size());
	DeviceContext->Unmap(PositionVertexBuffer, NULL);

	UINT stride = sizeof(v3);
	UINT offset = 0;

	SetDrawTopology(TopologyType);

	DeviceContext->IASetVertexBuffers(0, 1, &PositionVertexBuffer, &stride, &offset);
	DeviceContext->Draw((u32)PositionArray.size(), 0);
}

void dx11_renderer::SetViewport(float Width, float Height)
{
	D3D11_VIEWPORT vp;
	ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));
	vp.Width = Width;
	vp.Height = Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	DeviceContext->RSSetViewports(1, &vp);
}

void dx11_renderer::SetDrawTopology(draw_topology_types TopologyType)
{
	switch (TopologyType)
	{
		default:
		case draw_topology_types::TriangleList:
		{ DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); } break;

		case draw_topology_types::LineList:
		{ DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST); } break;

		case draw_topology_types::PointList:
		{ DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST); } break;
	}
}

void dx11_renderer::CompileShaderFromFile(LPCWSTR Filename, LPCSTR EntryPoint, shader_type ShaderType, void* ShaderRef)
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
	}

	hr = D3DCompileFromFile(Filename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint, Target.c_str(), dwShaderFlags, 0, &vsBlob, &pBlobError);
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
	}

	if (FAILED(hr))
	{
		vsBlob->Release();
		Assert(1 == 2);
	}

	vsBlob->Release();
}