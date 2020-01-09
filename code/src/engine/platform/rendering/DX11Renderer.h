#pragma once

class dx11_renderer
{
public:

	// Shared
	void Initialize(void* Window, int WindowWidth, int WindowHeight);
	void Cleanup();
	void FinishFrame();
	void Draw(std::vector<vertex>& VertexArray, draw_topology_types TopologyType);
	void Draw(std::vector<v3>& PositionArray, draw_topology_types TopologyType);
	void SetViewport(float Width, float Height);
	void SetDrawTopology(draw_topology_types TopologyType);
	void CompileShaderFromFile(std::string Filename, std::string EntryPoint, shader_type ShaderType, void* ShaderRef);
	void RegisterTexture(cAsset* Asset, bool GenerateMIPs);

	static matrix4x4 GetPerspectiveProjectionLH(bool Transpose, camera_info& CameraInfo);
	static matrix4x4 GetOrthographicProjectionLH(bool Transpose, camera_info& CameraInfo);
	static matrix4x4 GenerateViewMatrix(bool Transpose, camera_info& CameraInfo, v3& OutLookAtMatrix, bool OrthoUseMovement = false);

	HWND Window;

	// Specific
	ID3D11Device* Device = NULL;
	ID3D11DeviceContext* DeviceContext = NULL;
	ID3D11Debug* DebugDevice = NULL;
	IDXGISwapChain* Chain = NULL;
	ID3D11RenderTargetView* RenderTargetView = NULL;
	ID3D11SamplerState* DefaultSampler = NULL;
	ID3D11Texture2D* DepthStencilTex = NULL;
	ID3D11DepthStencilView* DepthStencilView = NULL;
	ID3D11DepthStencilState* DepthStencilEnabled = NULL;
	ID3D11DepthStencilState* DepthStencilDisabled = NULL;
	ID3D11BlendState* BlendState = NULL;

	ID3D11InputLayout* DefaultVertexLayout = NULL;
	ID3D11InputLayout* PositionVertexLayout = NULL;

	ID3D11VertexShader* MainVertexShader = NULL;
	ID3D11PixelShader* MainPixelShader = NULL;

	ID3D11Buffer* MainVertexBuffer;
	ID3D11Buffer* PositionVertexBuffer;
	ID3D11Buffer* ConstantBuffer;

private:



};

typedef dx11_renderer platform_renderer;