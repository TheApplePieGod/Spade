
#include "renderer.h"

#define NUM_CASCADES 3

ID3D11Buffer* MainBuffer;
ID3D11Buffer* ConstantBuffer;
ID3D11Buffer* ParaboloidConstants;
ID3D11Buffer* MaterialBuffer;
ID3D11Buffer* LightingBuffer;
shader_constants constants = shader_constants();
paraboloid_constants ParaboloidCs = paraboloid_constants();
material_shader_constants MaterialConstants = material_shader_constants(); // MOVE TO MEMORY
light_shader_constants LightConstants = light_shader_constants();

void Draw(ID3D11Buffer* buffer, cArray<vertex>& VertexArray, bool Triangles = true)
{
    D3D11_MAPPED_SUBRESOURCE Mapped;
    DeviceContext->Map(MainBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
    memcpy(Mapped.pData, VertexArray.GetRaw(), sizeof (vertex) * VertexArray.Num());
    DeviceContext->Unmap(MainBuffer, NULL);

    UINT stride = sizeof( vertex );
    UINT offset = 0;

    if (Triangles)
        DeviceContext->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    else
        DeviceContext->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_LINELIST );
    DeviceContext->IASetVertexBuffers( 0, 1, &buffer, &stride, &offset );
    DeviceContext->Draw(VertexArray.Num(), 0);
}

/*
* Creates DX texture based on asset params and adds an entry to the loaded texture registry (DATA MUST BE LOADED FIRST)
*/
s32 LoadAndRegisterTexture(cTextureAsset* asset)
{
    if (asset->Data != nullptr && asset->Loaded == true)
    {
        D3D11_SUBRESOURCE_DATA TextureData = {};        
        //TextureData.pSysMem = asset->Data;
        TextureData.SysMemPitch = asset->Channels * asset->Width;

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = asset->Width;
        desc.Height = asset->Height;
        desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        desc.MipLevels = 0;
        desc.ArraySize = 1;
        switch (asset->Channels)
        {
            case 0:
            {
                Assert(1==2);
            }
            case 1:
            {
                desc.Format = DXGI_FORMAT_A8_UNORM;
                break;
            }
            case 2:
            {
                Assert(1==2);
            } 
            case 3:
            case 4:
            {
                desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                break;
            }
        }
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT; // Only the GPU has access to read this resource
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0; // No CPU access is required

        ID3D11Texture2D *pTexture = NULL;
        HRESULT hr = Device->CreateTexture2D( &desc, NULL, &pTexture);
        if(FAILED(hr))
        {
            //return hr;
            return -1;
        }

        DeviceContext->UpdateSubresource( pTexture, 0, NULL, asset->Data, TextureData.SysMemPitch, 0 );

        texture_entry TexEntry;
        TexEntry.Asset = asset;
        TexEntry.TextureHandle = pTexture;
        TexEntry.Success = true;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 0;

        ID3D11ShaderResourceView* shader = NULL;
        hr = Device->CreateShaderResourceView((ID3D11Resource*)TexEntry.TextureHandle, NULL, &shader);
        if( FAILED( hr ) )
            Assert(1==2);

        DeviceContext->GenerateMips(shader);

        TexEntry.AssociatedShaderHandle = shader;

        return GlobalTextureRegistry.Add(TexEntry);
    }
    else
    {
        return -1;
    }
}

/*
* Should only ever have one channel atlas
*/
ID3D11ShaderResourceView* LoadFontAtlas(cFontAsset* asset, bool Mips = false)
{
    if (asset->Data != nullptr && asset->Loaded == true)
    {
        D3D11_SUBRESOURCE_DATA TextureData = {};        
        //TextureData.pSysMem = asset->Data;
        TextureData.SysMemPitch = asset->FontData.AtlasDim * 1; // assumes one channel

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = asset->FontData.AtlasDim;
        desc.Height = asset->FontData.AtlasDim;
        desc.MiscFlags = 0;
        desc.MipLevels = Mips ? 0 : 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_A8_UNORM; // should always be 1
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT; // Only the GPU has access to read this resource
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0; // No CPU access is required

        ID3D11Texture2D *pTexture = NULL;
        HRESULT hr = Device->CreateTexture2D( &desc, NULL, &pTexture);
        if(FAILED(hr))
        {
            //return hr;
            Assert(1==2);
        }

        DeviceContext->UpdateSubresource( pTexture, 0, NULL, asset->Data, TextureData.SysMemPitch, 0 );

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = Mips ? 0 : 1;

        ID3D11ShaderResourceView* shader = NULL;
        hr = Device->CreateShaderResourceView((ID3D11Resource*)pTexture, NULL, &shader);
        if( FAILED( hr ) )
            Assert(1==2);

        if (Mips)
            DeviceContext->GenerateMips(shader);

        pTexture->Release();
        return shader;
    }
    else
    {
        Assert(1==2); // fix
        return nullptr;
    }
}

void DXCleanup()
{
    for (u32 i = 0; i < GlobalTextureRegistry.Num(); i++)
    {
        SAFE_RELEASE(GlobalTextureRegistry[i].TextureHandle);
        SAFE_RELEASE(GlobalTextureRegistry[i].AssociatedShaderHandle);
    }
    for (int i = 0; i < NUM_CASCADES; i++)
    {
        SAFE_RELEASE(ShadowStencilView[i]);
    }
    for (int i = 0; i < MAX_POINTS * 2; i++)
    {
        SAFE_RELEASE(ParaboloidStencilViews[i]);
    }
    for (u32 i = 0; i < GlobalAssetRegistry.Num(); i++)
    {
        if (GlobalAssetRegistry[i]->Type == asset_type::Font)
        {
            SAFE_RELEASE(((cFontAsset*)GlobalAssetRegistry[i])->AtlasShaderHandle);
        }
    }

    SAFE_RELEASE(ParaboloidTex);
    SAFE_RELEASE(ParaboloidStencilTex);
    SAFE_RELEASE(ShadowStencilTex);
    SAFE_RELEASE(ShadowTex);

    //remove
    SAFE_RELEASE(MainBuffer);
    SAFE_RELEASE(ConstantBuffer);
    SAFE_RELEASE(MaterialBuffer);
    SAFE_RELEASE(LightingBuffer);
    SAFE_RELEASE(ParaboloidConstants);

    //shaders
    SAFE_RELEASE(MainVertexShader);
    SAFE_RELEASE(QuadGeometryShader);
    SAFE_RELEASE(QuadVertexShader);
    SAFE_RELEASE(MainPixelShader);
    SAFE_RELEASE(ParaboloidVertShader);
    SAFE_RELEASE(ParaboloidPixShader);
    SAFE_RELEASE(TwoDFontVertShader);
    SAFE_RELEASE(TwoDFontPixShader);

    SAFE_RELEASE(BlendState);
    SAFE_RELEASE(DepthStencilEnabled);
    SAFE_RELEASE(DepthStencilDisabled);
    SAFE_RELEASE(DepthStencilTex);
    SAFE_RELEASE(DepthStencilView);
    SAFE_RELEASE(RenderTargetView);
    SAFE_RELEASE(SamplerState1);
    SAFE_RELEASE(SamplerShadowClamp);
    SAFE_RELEASE(Chain);
    SAFE_RELEASE(DebugDevice);
    SAFE_RELEASE(DeviceContext);
    SAFE_RELEASE(Device);
 
	//DebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	//SAFE_RELEASE(DebugDevice);
}

void DXSetViewport(float Width, float Height)
{
    D3D11_VIEWPORT vp;
    ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));
    vp.Width = Width;
    vp.Height = Height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    DeviceContext->RSSetViewports( 1, &vp );
}

HRESULT InitDX(HWND Window, int BufferResHeight, int BufferResWidth)
{
    //
    // Device and swap chain
    //
    UINT MultiSamplingCount = 4;
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = BufferResWidth;
    sd.BufferDesc.Height = BufferResHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = Window;
    sd.SampleDesc.Count = MultiSamplingCount;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    
    const D3D_FEATURE_LEVEL lvl[] = {
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
    #ifdef SPADE_DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, lvl, _countof(lvl), D3D11_SDK_VERSION, &sd, &Chain, &Device, &FeatureLevelsSupported, &DeviceContext );
    if ( hr == E_INVALIDARG )
    {
        hr = D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, &lvl[1], _countof(lvl) - 1, D3D11_SDK_VERSION, &sd, &Chain, &Device, &FeatureLevelsSupported, &DeviceContext );
    }
    if (FAILED(hr))
       return hr;
    
    #ifdef SPADE_DEBUG
    //
    // Create debug device and supress certain warning messages
    //

    hr = Device->QueryInterface(__uuidof(ID3D11Debug), (void**) &DebugDevice);
    if(FAILED(hr))
        Assert(1==2);

    ID3D11InfoQueue* d3dInfoQueue = NULL;
    hr = Device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**) &d3dInfoQueue);
    if(FAILED(hr))
        Assert(1==2);

    D3D11_MESSAGE_ID hide [] =
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
    hr = Chain->GetBuffer(0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pRenderTargetTexture );
    if( FAILED( hr ) )
        return hr;

    hr = Device->CreateRenderTargetView( pRenderTargetTexture, NULL, &RenderTargetView );
    pRenderTargetTexture->Release();
    if( FAILED( hr ) )
        return hr;

    //DeviceContext->OMSetRenderTargets(1, &RenderTargetView, NULL); //set after depth stencil initialization


    RECT rc;
    GetClientRect( Window, &rc );
    D3D11_VIEWPORT vp;
    ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));
    vp.Width = (FLOAT)(rc.right - rc.left);
    vp.Height = (FLOAT)(rc.bottom - rc.top);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    DeviceContext->RSSetViewports( 1, &vp );


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
    if(FAILED(hr))
        Assert(1==2);

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

        hr = D3DCompileFromFile(L"../code/src/shader/mainshader.hlsl", NULL, NULL, "mainvs", "vs_5_0", dwShaderFlags, 0, &vsBlob, &pBlobError);
        if( FAILED( hr ) )
        {
            if( pBlobError != NULL )
            {
                OutputDebugStringA( (CHAR*)pBlobError->GetBufferPointer() );
                pBlobError->Release();
            }
            Assert(1==2);
        }

        hr = Device->CreateVertexShader( vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &MainVertexShader );
        if( FAILED( hr ) )
        {
            vsBlob->Release();
            Assert(1==2);
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

        ID3D11InputLayout* VertLayout = NULL;

        hr = Device->CreateInputLayout(layout, 
                                        ArrayCount(layout),
                                        vsBlob->GetBufferPointer(),
                                        vsBlob->GetBufferSize(),
                                        &VertLayout);
        if( FAILED( hr ) )
        {
            vsBlob->Release();
            Assert(1==2);
        }

        DeviceContext->IASetInputLayout(VertLayout);
        DeviceContext->VSSetShader(MainVertexShader, NULL, 0);

        VertLayout->Release();
        vsBlob->Release();

        // PIXEL SHADER
        pBlobError = NULL;
        ID3DBlob* psBlob = NULL; // output

        hr = D3DCompileFromFile(L"../code/src/shader/mainshader.hlsl", NULL, NULL, "mainps", "ps_5_0", dwShaderFlags, 0, &psBlob, &pBlobError);
        if( FAILED( hr ) )
        {
            if( pBlobError != NULL )
            {
                OutputDebugStringA( (CHAR*)pBlobError->GetBufferPointer() );
                pBlobError->Release();
            }
            Assert(1==2);
        }

        hr = Device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &MainPixelShader );
        if( FAILED( hr ) )
        {
            psBlob->Release();
            Assert(1==2);
        }

        DeviceContext->PSSetShader(MainPixelShader, NULL, 0);

        psBlob->Release();

        // GEO SHADER
        pBlobError = NULL;
        ID3DBlob* gsBlob = NULL; // output

        hr = D3DCompileFromFile(L"../code/src/shader/geometryshader.hlsl", NULL, NULL, "quadgs", "gs_5_0", dwShaderFlags, 0, &gsBlob, &pBlobError);
        if( FAILED( hr ) )
        {
            if( pBlobError != NULL )
            {
                OutputDebugStringA( (CHAR*)pBlobError->GetBufferPointer() );
                pBlobError->Release();
            }
            Assert(1==2);
        }

        hr = Device->CreateGeometryShader( gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), NULL, &QuadGeometryShader );
        if( FAILED( hr ) )
        {
            gsBlob->Release();
            Assert(1==2);
        }

        gsBlob->Release();
    }


    //
    // compile paraboloid shadow mapping shaders
    //
    {
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
        
        // VERT SHADER
        ID3DBlob* pBlobError = NULL;
        ID3DBlob* vsBlob = NULL; // output

        hr = D3DCompileFromFile(L"../code/src/shader/paraboloidshadowing.hlsl", NULL, NULL, "mainvs", "vs_5_0", dwShaderFlags, 0, &vsBlob, &pBlobError);
        if( FAILED( hr ) )
        {
            if( pBlobError != NULL )
            {
                OutputDebugStringA( (CHAR*)pBlobError->GetBufferPointer() );
                pBlobError->Release();
            }
            Assert(1==2);
        }

        hr = Device->CreateVertexShader( vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &ParaboloidVertShader );
        if( FAILED( hr ) )
        {
            vsBlob->Release();
            Assert(1==2);
        }

        vsBlob->Release();

        // PIXEL SHADER
        pBlobError = NULL;
        ID3DBlob* psBlob = NULL; // output

        hr = D3DCompileFromFile(L"../code/src/shader/paraboloidshadowing.hlsl", NULL, NULL, "mainps", "ps_5_0", dwShaderFlags, 0, &psBlob, &pBlobError);
        if( FAILED( hr ) )
        {
            if( pBlobError != NULL )
            {
                OutputDebugStringA( (CHAR*)pBlobError->GetBufferPointer() );
                pBlobError->Release();
            }
            Assert(1==2);
        }

        hr = Device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &ParaboloidPixShader );
        if( FAILED( hr ) )
        {
            psBlob->Release();
            Assert(1==2);
        }

        psBlob->Release();
    }

    //
    // compile geo shader
    //
    {
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
        
        // VERT SHADER
        ID3DBlob* pBlobError = NULL;
        ID3DBlob* vsBlob = NULL; // output

        hr = D3DCompileFromFile(L"../code/src/shader/geometryshader.hlsl", NULL, NULL, "quadvs", "vs_5_0", dwShaderFlags, 0, &vsBlob, &pBlobError);
        if( FAILED( hr ) )
        {
            if( pBlobError != NULL )
            {
                OutputDebugStringA( (CHAR*)pBlobError->GetBufferPointer() );
                pBlobError->Release();
            }
            Assert(1==2);
        }

        hr = Device->CreateVertexShader( vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &QuadVertexShader );
        if( FAILED( hr ) )
        {
            vsBlob->Release();
            Assert(1==2);
        }

        vsBlob->Release();

        // // PIXEL SHADER
        // pBlobError = NULL;
        // ID3DBlob* psBlob = NULL; // output

        // hr = D3DCompileFromFile(L"../code/src/shader/geometryshader.hlsl", NULL, NULL, "quadps", "ps_5_0", dwShaderFlags, 0, &psBlob, &pBlobError);
        // if( FAILED( hr ) )
        // {
        //     if( pBlobError != NULL )
        //     {
        //         OutputDebugStringA( (CHAR*)pBlobError->GetBufferPointer() );
        //         pBlobError->Release();
        //     }
        //     Assert(1==2);
        // }

        // hr = Device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &QuadPixelShader );
        // if( FAILED( hr ) )
        // {
        //     psBlob->Release();
        //     Assert(1==2);
        // }

        // psBlob->Release();
    }

    //
    // compile 2D font rendering shaders
    //
    {
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
        
        // VERT SHADER
        ID3DBlob* pBlobError = NULL;
        ID3DBlob* vsBlob = NULL; // output

        hr = D3DCompileFromFile(L"../code/src/shader/2dfontshader.hlsl", NULL, NULL, "mainvs", "vs_5_0", dwShaderFlags, 0, &vsBlob, &pBlobError);
        if( FAILED( hr ) )
        {
            if( pBlobError != NULL )
            {
                OutputDebugStringA( (CHAR*)pBlobError->GetBufferPointer() );
                pBlobError->Release();
            }
            Assert(1==2);
        }

        hr = Device->CreateVertexShader( vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &TwoDFontVertShader );
        if( FAILED( hr ) )
        {
            vsBlob->Release();
            Assert(1==2);
        }

        vsBlob->Release();

        // PIXEL SHADER
        pBlobError = NULL;
        ID3DBlob* psBlob = NULL; // output

        hr = D3DCompileFromFile(L"../code/src/shader/2dfontshader.hlsl", NULL, NULL, "mainps", "ps_5_0", dwShaderFlags, 0, &psBlob, &pBlobError);
        if( FAILED( hr ) )
        {
            if( pBlobError != NULL )
            {
                OutputDebugStringA( (CHAR*)pBlobError->GetBufferPointer() );
                pBlobError->Release();
            }
            Assert(1==2);
        }

        hr = Device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &TwoDFontPixShader );
        if( FAILED( hr ) )
        {
            psBlob->Release();
            Assert(1==2);
        }

        psBlob->Release();
    }

    //
    // Create main vertex buffer
    //
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.Usage            = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth        = sizeof( vertex ) * 100000; // MAX BUFFER SIZE
    bufferDesc.BindFlags        = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;

    hr = Device->CreateBuffer( &bufferDesc, NULL, &MainBuffer );
    if ( FAILED (hr))
        Assert(1==2);;


    //
    // Create main constant buffer
    //
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.Usage            = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth        = sizeof( shader_constants ); // CONST BUFFER SIZE
    bufferDesc.BindFlags        = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;

    hr = Device->CreateBuffer( &bufferDesc, NULL, &ConstantBuffer );
    if ( FAILED (hr))
        Assert(1==2);

    DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);

    //
    // Create paraboloid constant buffer
    //
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.Usage            = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth        = sizeof( paraboloid_constants ); // CONST BUFFER SIZE
    bufferDesc.BindFlags        = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;

    hr = Device->CreateBuffer( &bufferDesc, NULL, &ParaboloidConstants );
    if ( FAILED (hr))
        Assert(1==2);

    //
    // Create material constant buffer
    //
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.Usage            = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth        = sizeof( material_shader_constants ); // CONST BUFFER SIZE
    bufferDesc.BindFlags        = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;

    hr = Device->CreateBuffer( &bufferDesc, NULL, &MaterialBuffer );
    if ( FAILED (hr))
        Assert(1==2);

    DeviceContext->PSSetConstantBuffers(0, 1, &MaterialBuffer);


    //
    // Create lighting constant buffer + map it
    //
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.Usage            = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth        = sizeof( light_shader_constants ); // CONST BUFFER SIZE
    bufferDesc.BindFlags        = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;

    hr = Device->CreateBuffer( &bufferDesc, NULL, &LightingBuffer );
    if ( FAILED (hr))
        Assert(1==2);

    DeviceContext->PSSetConstantBuffers(1, 1, &LightingBuffer);

    D3D11_MAPPED_SUBRESOURCE Mapped;
    DeviceContext->Map(LightingBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
    light_shader_constants* dataPtr = (light_shader_constants*)Mapped.pData;
    *dataPtr = LightConstants;
    DeviceContext->Unmap(LightingBuffer, NULL);

    //
    // create texture sampler
    // NOTE: If you do not set a samplerstate, the default will be MIN_MAG_MIP_LINEAR
    //
    D3D11_SAMPLER_DESC SamplerDesc;
    SamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC; //D3D11_FILTER_ANISOTROPIC;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // tile
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.MipLODBias = 0;
    SamplerDesc.MaxAnisotropy = 16;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SamplerDesc.BorderColor[0] = 1.0f;
    SamplerDesc.BorderColor[1] = 1.0f;
    SamplerDesc.BorderColor[2] = 1.0f;
    SamplerDesc.BorderColor[3] = 1.0f;
    SamplerDesc.MinLOD = 0;
    SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    Device->CreateSamplerState(&SamplerDesc, &SamplerState1);
    DeviceContext->PSSetSamplers( 0, 1, &SamplerState1);

    //
    // shadow clamp sampler
    //
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	Device->CreateSamplerState(&SamplerDesc, &SamplerShadowClamp);
    DeviceContext->PSSetSamplers( 1, 1, &SamplerShadowClamp);


    //
    // Setup directional shadow map resource
    //
    {
    D3D11_TEXTURE2D_DESC descDepth1;
    ZeroMemory(&descDepth1, sizeof(descDepth1));
    descDepth1.Width = 2048;
    descDepth1.Height = 2048;
    descDepth1.MipLevels = 0;
    descDepth1.ArraySize = NUM_CASCADES;
    descDepth1.Format = DXGI_FORMAT_R32_TYPELESS;
    descDepth1.SampleDesc.Count = 1;
    descDepth1.SampleDesc.Quality = 0;
    descDepth1.Usage = D3D11_USAGE_DEFAULT;
    descDepth1.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    descDepth1.CPUAccessFlags = 0;
    descDepth1.MiscFlags = 0;
    hr = Device->CreateTexture2D( &descDepth1, NULL, &ShadowStencilTex);
    if( FAILED( hr ) )
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDescc;
    srvDescc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDescc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDescc.Texture2DArray.MipLevels = 1;
    srvDescc.Texture2DArray.MostDetailedMip = 0;
    srvDescc.Texture2DArray.FirstArraySlice = 0;
    srvDescc.Texture2DArray.ArraySize = NUM_CASCADES;

    hr = Device->CreateShaderResourceView((ID3D11Resource*)ShadowStencilTex, &srvDescc, &ShadowTex);
    if( FAILED( hr ) )
        Assert(1==2);
    }
    for (int i = 0; i < NUM_CASCADES; i++)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV2;
        ZeroMemory(&descDSV2, sizeof(descDSV2));
        descDSV2.Format = DXGI_FORMAT_D32_FLOAT;
        descDSV2.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        descDSV2.Texture2DArray.MipSlice = 0;
        descDSV2.Texture2DArray.ArraySize = 1;
        descDSV2.Texture2DArray.FirstArraySlice = i;

        hr = Device->CreateDepthStencilView( ShadowStencilTex, &descDSV2, &ShadowStencilView[i]);
        if( FAILED( hr ) )
            return hr;
    }

    //
    // Setup point light shadow map resource
    //
    {
    D3D11_TEXTURE2D_DESC descDepth1;
    ZeroMemory(&descDepth1, sizeof(descDepth1));
    descDepth1.Width = 1024; // point light resolution
    descDepth1.Height = 1024;
    descDepth1.MipLevels = 0;
    descDepth1.ArraySize = MAX_POINTS * 2; 
    descDepth1.Format = DXGI_FORMAT_R32_TYPELESS ;
    descDepth1.SampleDesc.Count = 1;
    descDepth1.SampleDesc.Quality = 0;
    descDepth1.Usage = D3D11_USAGE_DEFAULT;
    descDepth1.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    descDepth1.CPUAccessFlags = 0;
    descDepth1.MiscFlags = 0;
    hr = Device->CreateTexture2D( &descDepth1, NULL, &ParaboloidStencilTex);
    if( FAILED( hr ) )
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDescc;
    srvDescc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDescc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDescc.Texture2DArray.MipLevels = 1;
    srvDescc.Texture2DArray.MostDetailedMip = 0;
    srvDescc.Texture2DArray.FirstArraySlice = 0;
    srvDescc.Texture2DArray.ArraySize = MAX_POINTS * 2;

    hr = Device->CreateShaderResourceView((ID3D11Resource*)ParaboloidStencilTex, &srvDescc, &ParaboloidTex);
    if( FAILED( hr ) )
        Assert(1==2);

    }
    for (int i = 0; i < MAX_POINTS * 2; i++)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV2;
        ZeroMemory(&descDSV2, sizeof(descDSV2));
        descDSV2.Format = DXGI_FORMAT_D32_FLOAT;
        descDSV2.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        descDSV2.Texture2DArray.MipSlice = 0;
        descDSV2.Texture2DArray.ArraySize = 1;
        descDSV2.Texture2DArray.FirstArraySlice = i;

        hr = Device->CreateDepthStencilView( ParaboloidStencilTex, &descDSV2, &ParaboloidStencilViews[i]);
        if( FAILED( hr ) )
            return hr;
    }

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
    descDepth.Width = BufferResWidth;
    descDepth.Height = BufferResHeight;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format =DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = MultiSamplingCount;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

    hr = Device->CreateTexture2D( &descDepth, NULL, &DepthStencilTex);
    if( FAILED( hr ) )
        return hr;


    //
    // Setup depth view
    //
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));

    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    descDSV.Texture2D.MipSlice = 0;

    hr = Device->CreateDepthStencilView( DepthStencilTex, &descDSV, &DepthStencilView);
    if( FAILED( hr ) )
        return hr;

    DeviceContext->OMSetRenderTargets( 1, &RenderTargetView, DepthStencilView );


    //
    // Create two depth stencil states
    //
    D3D11_DEPTH_STENCIL_DESC DepthDesc = {};
    DepthDesc.DepthEnable = true;
    DepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    DepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
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
    if( FAILED( hr ) )
        return hr;
    
    DepthDesc.DepthEnable = false;

    hr = Device->CreateDepthStencilState(&DepthDesc, &DepthStencilDisabled);
    if( FAILED( hr ) )
        return hr;

    DeviceContext->OMSetDepthStencilState(DepthStencilEnabled, 1);

    return hr;
}

void RenderTextAtPosition(const char* Text, v2 Position, v2 Scale, float Rotation, v4 Color, cFontAsset* Font)
{
    char String[256];
    _snprintf_s(String, sizeof(String), "%s", Text);
    cArray<vertex> Quad;
    v2 AddedPosition = V2(0, 0);
    u32 LastChar = '\0';

    for (u32 x = 0; String[x] != '\0'; x++)
    {
        float kern = 0;
        float lsb = 0;
        u32 AsciiVal = String[x];
        char_entry CharData = Font->FindCharEntryByAscii(AsciiVal);

        for (u32 i = 0; i < Font->KernValues.Num(); i++)
        {
            if (Font->KernValues[i].AsciiVal1 == LastChar && Font->KernValues[i].AsciiVal2 == AsciiVal)
            {
                kern = (f32)Font->KernValues[i].Spacing * Scale.x;
                break;
            }
        }
        if (CharData.LeftSideBearing != CharData.OffsetX)
            lsb = (CharData.LeftSideBearing + (f32)CharData.OffsetX) * Scale.x;
        else
            lsb = CharData.LeftSideBearing * Scale.x;

        v2 Size = v2{(f32)CharData.Width * Scale.x, (f32)CharData.Height * Scale.y};
        v2 BottomLeft = Position + AddedPosition + V2(kern + lsb, Font->FontData.Ascent - ((f32)CharData.OffsetY + CharData.Height) * Scale.y);
        v2 TopLeft = v2{BottomLeft.x, BottomLeft.y + Size.y};
        v2 BottomRight = v2{BottomLeft.x + Size.x, BottomLeft.y};
        v2 TopRight = v2{BottomLeft.x + Size.x, BottomLeft.y + Size.y};
        v2 TopLeftUV = V2((float)CharData.TopLeftOffsetX / (float)Font->FontData.AtlasDim, (float)CharData.TopLeftOffsetY / (float)Font->FontData.AtlasDim);
        v2 BottomRightUV = V2((float)(CharData.TopLeftOffsetX + CharData.Width) / (float)Font->FontData.AtlasDim, (float)(CharData.TopLeftOffsetY + CharData.Height) / (float)Font->FontData.AtlasDim);

        Quad.Add(TEXVERTEX(BottomLeft.x, BottomLeft.y, 1, TopLeftUV.u, BottomRightUV.v), true); // bottom left
        Quad.Add(TEXVERTEX(TopLeft.x, TopLeft.y, 1, TopLeftUV.u, TopLeftUV.v), true); // top left
        Quad.Add(TEXVERTEX(BottomRight.x, BottomRight.y, 1, BottomRightUV.u, BottomRightUV.v), true); // bottom right
        Quad.Add(TEXVERTEX(TopLeft.x, TopLeft.y, 1, TopLeftUV.u, TopLeftUV.v), true); // top left
        Quad.Add(TEXVERTEX(TopRight.x, TopRight.y, 1, BottomRightUV.u, TopLeftUV.v), true); // top right
        Quad.Add(TEXVERTEX(BottomRight.x, BottomRight.y, 1, BottomRightUV.u, BottomRightUV.v), true); // bottom right

        AddedPosition += V2((CharData.AdvanceWidth * Scale.x) + kern, 0);
        LastChar = String[x];
    }

    DirectX::XMMATRIX translation, rotationz, scale;
    translation = DirectX::XMMatrixTranslation(0, 0, 0);
    rotationz = DirectX::XMMatrixRotationZ(Rotation * (Pi32 / 180.0f));
    scale = DirectX::XMMatrixScaling(1, 1, 1);

    constants.World = scale * rotationz * translation; // transform order: scale, rotate (degrees), translate
    constants.color = Color;
    constants.has_texture = true;
    constants.padding = true; //is font

    D3D11_MAPPED_SUBRESOURCE Mapped;
    DeviceContext->Map(ConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
    shader_constants* dataPtr = (shader_constants*)Mapped.pData;
    *dataPtr = constants;
    DeviceContext->Unmap(ConstantBuffer, NULL);

    Draw(MainBuffer, Quad);
    Quad.ManualFree();
}

//assumes shader resource for the texture is set prior to function call
void RenderQuad(bool HasTexture, v2 Position, v2 Scale, v2 Extent, v4 Tint = colors::White)
{
    cArray<vertex> Quad;
    v2 Size = v2{Extent.x * Scale.x, Extent.y * Scale.y};
    v2 BottomLeft = Position;
    v2 TopLeft = v2{BottomLeft.x, BottomLeft.y + Size.y};
    v2 BottomRight = v2{BottomLeft.x + Size.x, BottomLeft.y};
    v2 TopRight = v2{BottomLeft.x + Size.x, BottomLeft.y + Size.y};
    v2 TopLeftUV = V2(0, 0);
    v2 BottomRightUV = V2(1, 1);

    Quad.Add(TEXVERTEX(BottomLeft.x, BottomLeft.y, 1, TopLeftUV.u, BottomRightUV.v)); // bottom left
    Quad.Add(TEXVERTEX(TopLeft.x, TopLeft.y, 1, TopLeftUV.u, TopLeftUV.v)); // top left
    Quad.Add(TEXVERTEX(BottomRight.x, BottomRight.y, 1, BottomRightUV.u, BottomRightUV.v)); // bottom right
    Quad.Add(TEXVERTEX(TopLeft.x, TopLeft.y, 1, TopLeftUV.u, TopLeftUV.v)); // top left
    Quad.Add(TEXVERTEX(TopRight.x, TopRight.y, 1, BottomRightUV.u, TopLeftUV.v)); // top right
    Quad.Add(TEXVERTEX(BottomRight.x, BottomRight.y, 1, BottomRightUV.u, BottomRightUV.v)); // bottom right

    DirectX::XMMATRIX translation, rotationz, scale;
    translation = DirectX::XMMatrixTranslation(0, 0, 10);
    rotationz = DirectX::XMMatrixRotationZ(0 * (Pi32 / 180.0f));
    scale = DirectX::XMMatrixScaling(1, 1, 1);

    constants.World = scale * rotationz * translation; // transform order: scale, rotate (degrees), translate
    constants.color = Tint;
    constants.has_texture = HasTexture;
    constants.padding = false; // is font

    D3D11_MAPPED_SUBRESOURCE Mapped;
    DeviceContext->Map(ConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
    shader_constants* dataPtr = (shader_constants*)Mapped.pData;
    *dataPtr = constants;
    DeviceContext->Unmap(ConstantBuffer, NULL);

    Draw(MainBuffer, Quad);
    Quad.ManualFree();
}