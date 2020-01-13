#pragma once

class dx11_renderer
{
public:

	// Shared
	void Initialize(void* Window, int WindowWidth, int WindowHeight);
	void Cleanup();
	void FinishFrame();
	void Draw(vertex* InVertexArray, u32 NumVertices, draw_topology_types TopologyType);
	void Draw(v3* InPositionArray, u32 NumVertices, draw_topology_types TopologyType);
	void SetViewport(float Width, float Height);
	void SetDrawTopology(draw_topology_types TopologyType);
	void CompileShaderFromFile(std::string Filename, std::string EntryPoint, shader_type ShaderType, void* ShaderRef);
	void RegisterTexture(cAsset* Asset, bool GenerateMIPs);
	void BindMaterial(const material& InMaterial);
	void MapConstants(constants_type Type);

	/* Generates world matrix & maps the constants
	 * Call after other fields are set
	*/
	void MapActorConstants(actor_component& InActor, const rendering_component& InComponent);

	static matrix4x4 GetPerspectiveProjectionLH(bool Transpose, camera_info CameraInfo);
	static matrix4x4 GetOrthographicProjectionLH(bool Transpose, camera_info CameraInfo);
	static matrix4x4 GenerateViewMatrix(bool Transpose, camera_info CameraInfo, v3& OutLookAtMatrix, bool OrthoUseMovement = true);

	HWND Window;

	// Specific
	template<typename AssociatedStruct>
	void MapBuffer(void* BufferRef, AssociatedStruct Data);

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

	ID3D11Buffer* FrameConstantBuffer;
	ID3D11Buffer* ActorConstantBuffer;
	ID3D11Buffer* MaterialConstantBuffer;

private:

	inline matrix4x4
	ToMatrix4x4(const DirectX::XMMATRIX& A) // assumes transposition
	{
		matrix4x4 out;
		DirectX::XMFLOAT4X4 Res;
		DirectX::XMStoreFloat4x4(&Res, DirectX::XMMatrixTranspose(A));

		out.m11 = Res._11;
		out.m12 = Res._12;
		out.m13 = Res._13;
		out.m14 = Res._14;
		out.m21 = Res._21;
		out.m22 = Res._22;
		out.m23 = Res._23;
		out.m24 = Res._24;
		out.m31 = Res._31;
		out.m32 = Res._32;
		out.m33 = Res._33;
		out.m34 = Res._34;
		out.m41 = Res._41;
		out.m42 = Res._42;
		out.m43 = Res._43;
		out.m44 = Res._44;

		return out;
	}

};

typedef dx11_renderer platform_renderer;