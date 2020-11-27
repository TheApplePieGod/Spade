#pragma once

class dx11_renderer
{
public:

	// Shared
	void Initialize(void* Window, int WindowWidth, int WindowHeight);
	void Cleanup();
	void FinishFrame();
	void SetRendererState(render_state State);
	void* GetShaderResource();
	void Draw(vertex* InVertexArray, u32 NumVertices, draw_topology_type TopologyType);
	void Draw(v3* InPositionArray, u32 NumVertices, draw_topology_type TopologyType);
	void DrawInstanced(vertex* InVertexArray, u32 NumVertices, u32 NumInstances, draw_topology_type TopologyType);
	void DrawIndexedInstanced(vertex* InVertexArray, u32* InIndexArray, u32 NumVertices, u32 NumIndices, u32 IndexOffset, u32 NumInstances, draw_topology_type TopologyType);
	void DrawIndexedTerrainChunk(vertex* InVertexArray, u32* InIndexArray, u32 NumVertices, u32 NumIndices); // assumes triangle list w 4 vertices and 6 indices
	void SetViewport(float Width, float Height);
	void SetDrawTopology(draw_topology_type TopologyType);
	void CompileShaderFromFile(std::string Filename, std::string EntryPoint, shader_type ShaderType, void* ShaderRef);
	void RegisterTexture(cAsset* Asset, bool GenerateMIPs);
	void BindMaterial(const material& InMaterial);
	void MapConstants(map_operation Type);
	// 6 ids: left, right, up, down, front, back. Textures must be uniform size & channels
	void UpdateSkybox(s32* TextureIDs);
	void UpdateLandscapeTextures(s32* TextureIDs, int Count);
	void SetPipelineState(const pipeline_state& InState);

	static matrix4x4 GetPerspectiveProjectionLH(bool Transpose, camera_info CameraInfo);
	static matrix4x4 GetOrthographicProjectionLH(bool Transpose, camera_info CameraInfo);
	static matrix4x4 GetOrthographicProjectionOffCenterLH(bool Transpose, camera_info CameraInfo, v2 Min, v2 Max);
	static matrix4x4 GenerateViewMatrix(bool Transpose, camera_info CameraInfo, v3& OutLookAtVector, v3& OutUpVector, bool OrthoUseMovement = true, v3 InLookAtVector = {0.f,0.f,0.f});
	static matrix4x4 GeneratePlanetaryViewMatrix(bool Transpose, camera_info CameraInfo, v2 MouseDelta, v3 ForwardVector, v3& OutLookAtVector, v3& OutUpVector, bool OrthoUseMovement = true);
	static matrix4x4 GenerateWorldMatrix(transform Transform);
	static matrix4x4 InverseMatrix(const matrix4x4& Matrix, bool Transpose);
	static v3 GetWorldSpaceDirectionFromMouse(v2 MousePos, camera* Camera);

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
	ID3D11SamplerState* ClampSampler = NULL;
	ID3D11Texture2D* DepthStencilTex = NULL;
	ID3D11ShaderResourceView* DepthStencilResource = NULL;
	ID3D11DepthStencilView* DepthStencilView = NULL;
	ID3D11DepthStencilState* DepthStencilEnabled = NULL;
	ID3D11DepthStencilState* DepthStencilDisabled = NULL;
	ID3D11RasterizerState* DefaultCullBackface = NULL;
	ID3D11RasterizerState* DefaultCullFrontface = NULL;
	ID3D11RasterizerState* DefaultCullNone = NULL;
	ID3D11RasterizerState* Wireframe = NULL;
	ID3D11BlendState* BlendState = NULL;

	ID3D11InputLayout* DefaultVertexLayout = NULL;
	ID3D11InputLayout* PositionVertexLayout = NULL;

	ID3D11Buffer* MainIndexBuffer = NULL;
	ID3D11Buffer* TerrainChunkIndexBuffer = NULL;

	ID3D11Buffer* MainVertexBuffer = NULL;
	ID3D11Buffer* PositionVertexBuffer = NULL;
	ID3D11Buffer* TerrainChunkVertexBuffer = NULL;

	ID3D11Buffer* FrameConstantBuffer = NULL;
	ID3D11Buffer* ActorConstantBuffer = NULL;
	ID3D11Buffer* MaterialConstantBuffer = NULL;
	ID3D11Buffer* LightingConstantBuffer = NULL;

	ID3D11ShaderResourceView* SkyboxCube = NULL;
	ID3D11ShaderResourceView* LandscapeTextureArray = NULL;

	// Shadow mapping
	ID3D11Texture2D* ShadowMapTex = NULL;
	ID3D11DepthStencilView* ShadowMapView = NULL;
	ID3D11ShaderResourceView* ShadowMapResource = NULL;

	ID3D11Texture2D* VarianceMapTex = NULL;
	ID3D11RenderTargetView* VarianceMapView = NULL;
	ID3D11ShaderResourceView* VarianceMapResource = NULL;

private:

	inline static matrix4x4
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

	inline static DirectX::XMMATRIX
	ToDXM(const matrix4x4& A)
	{
		DirectX::XMMATRIX Res;
		Res = DirectX::XMMatrixSet(
			A.m11,
			A.m12,
			A.m13,
			A.m14,
			A.m21,
			A.m22,
			A.m23,
			A.m24,
			A.m31,
			A.m32,
			A.m33,
			A.m34,
			A.m41,
			A.m42,
			A.m43,
			A.m44
		);

		return Res;
	}

};

typedef dx11_renderer platform_renderer;