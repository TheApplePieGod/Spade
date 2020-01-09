#if !defined(RENDERER_H)

global_var ID3D11Device* Device = NULL;
global_var ID3D11DeviceContext* DeviceContext = NULL;
global_var ID3D11Debug* DebugDevice = NULL;
global_var IDXGISwapChain* Chain = NULL;
global_var ID3D11RenderTargetView* RenderTargetView = NULL;
global_var ID3D11SamplerState* AnisotropicSampler = NULL;
global_var ID3D11SamplerState* SamplerShadowClamp = NULL;
global_var ID3D11SamplerState* PointSampler = NULL;
global_var ID3D11Texture2D* DepthStencilTex = NULL;
global_var ID3D11DepthStencilView* DepthStencilView = NULL; 
global_var ID3D11DepthStencilState* DepthStencilEnabled = NULL;
global_var ID3D11DepthStencilState* DepthStencilDisabled = NULL;
global_var ID3D11BlendState* BlendState = NULL;

global_var ID3D11VertexShader* MainVertexShader = NULL;
global_var ID3D11GeometryShader* QuadGeometryShader = NULL;
global_var ID3D11VertexShader* QuadVertexShader = NULL;
global_var ID3D11VertexShader* ParaboloidVertShader = NULL;
global_var ID3D11VertexShader* TwoDFontVertShader = NULL;

global_var ID3D11PixelShader* MainPixelShader = NULL;
global_var ID3D11PixelShader* QuadPixelShader = NULL;
global_var ID3D11PixelShader* ParaboloidPixShader = NULL;
global_var ID3D11PixelShader* TwoDFontPixShader = NULL;

// directional light mapping
global_var ID3D11ShaderResourceView* ShadowTex;
global_var ID3D11Texture2D* ShadowStencilTex;
global_var ID3D11DepthStencilView* ShadowStencilView[3]; 

// point light mapping
global_var ID3D11ShaderResourceView* ParaboloidTex;
global_var ID3D11Texture2D* ParaboloidStencilTex;
global_var ID3D11DepthStencilView* ParaboloidStencilViews[MAX_POINTS * 2];

vertex verticesCombo[] =
{
    // triangle
    VERTEX(0.0f, 1.f, 1.f, 0.0f, 0.0f, 0.5f, 1.0f),
    VERTEX(1.f, -1.f, 1.f, 0.5f, 0.0f, 0.0f, 1.0f),
    VERTEX(-1.f, -1.f, 1.f, 0.0f, 0.5f, 0.0f, 1.0f)

    // TEXVERTEX(-1.0f, 1.f, 1.f, 0.f, 0.f),
    // TEXVERTEX(1.f, -1.f, 1.f, 1.f, 1.f),
    // TEXVERTEX(-1.f, -1.f, 1.f, 0.0f, 0.5f, 0.0f, 1.0f, 0.f, 1.f),
    // TEXVERTEX(1.0f, 1.f, 1.f, 0.0f, 0.0f, 0.5f, 1.0f, 1.f, 0.f),
    // TEXVERTEX(1.f, -1.f, 1.f, 0.5f, 0.0f, 0.0f, 1.0f, 1.f, 1.f),
    // TEXVERTEX(-1.f, 1.f, 1.f, 0.0f, 0.5f, 0.0f, 1.0f, 0.f, 0.f),

    // cube
    // VERTEX(-5.f, -5.f, -5.f, 0.0f, 0.0f, 0.0f, 0.0f),
    // VERTEX(-5.f, -5.f, 5.f, 0.5f, 0.0f, 0.0f, 1.0f),
    // VERTEX(-5.f, 5.f, -5.f, 0.0f, 0.0f, 1.0f, 0.0f),
    // VERTEX(-5.f, 5.f, 5.f, 0.0f, 0.0f, 1.0f, 1.0f),
    // VERTEX(5.f, -5.f, -5.f, 0.0f, 1.0f, 0.0f, 0.0f),
    // VERTEX(5.f, -5.f, 5.f, 0.0f, 1.0f, 0.0f, 1.0f),
    // VERTEX(5.f, 5.f, -5.f, 0.5f, 1.0f, 1.0f, 0.0f),
    // VERTEX(5.f, 5.f, 5.f, 0.0f, 1.0f, 1.0f, 1.0f),
    // VERTEX(-5.f, -5.f, -5.f, 0.0f, 0.0f, 0.0f, 0.0f),
};

#define RENDERER_H
#endif