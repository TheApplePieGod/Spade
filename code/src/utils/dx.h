#if !defined(DX_H)

#define MAX_LIGHTS 8
#define MAX_POINTS 50

struct vertex
{
    // Position in x/y plane
    f32 x, y, z;

    f32 r,g,b,a;

    // UV coordinates
    f32 u, v;

    //normals
    f32 nx, ny, nz;


};

struct shader_constants
{
    matrix4x4 World;
    matrix4x4 ViewProj;
    //matrix4x4 test;
    //matrix4x4 projection;
    //matrix4x4 view;
    v3 RenderLocation;
    u32 padding;
    v3 Scale;
    u32 has_texture = 0;
    v4        color;
};

struct paraboloid_constants
{
    matrix4x4 LightViewMatrix;
    f32 NearPlane;
    f32 FarPlane;
    f32 Direction;
    f32 padding;
    v3 RenderLocation;
    f32 padding2;
    v3 Scale;
    f32 padding3;
};

struct light_shader_input
{
    v3      Position;               // 16 bytes
    f32 Intensity = 1.f;
    //----------------------------------- (16 byte boundary)
    v3      Direction;              // 16 bytes
    int Padding2;
    //----------------------------------- (16 byte boundary)
    v4      Color;                  // 16 bytes
    //----------------------------------- (16 byte boundary)
    f32       SpotAngle;              // 4 bytes
    f32       ConstantAttenuation;    // 4 bytes
    f32       LinearAttenuation;      // 4 bytes
    f32       QuadraticAttenuation;   // 4 bytes
    //----------------------------------- (16 byte boundary)
    int         LightType;               // 4 bytes
    b32        Enabled; 
    b32        CastShadow;               // 4 bytes
    int        Padding3;                // 12 bytes
    //----------------------------------- (16 byte boundary)
    matrix4x4 ViewMatrix;
    matrix4x4 ProjectionMatrix;
};

struct light_shader_constants // separate lights and materials
{    
    v4 EyePosition; // general camera location for specular
    // lights properties
    light_shader_input Lights[MAX_LIGHTS];
    v4 GlobalAmbient;
    matrix4x4 CascadeMatrices[3];
    u32 UsesDirectional;
    v3 Padding;
};

struct material_shader_constants
{
    // material properties
    //float3 Normal; // implement normals and texture normals
    v4 DiffuseColor;
    u32 UsesTextureDiffuse;
    f32 SpecularPower;
    v2 padding;
};

struct mesh_render_resources
{
    cArray<vertex> Vertices;
    u32 ShaderID;
    v3 BoundingBoxMin;
    v3 BoundingBoxMax;
};

vertex VERTEX(f32 X, f32 Y, f32 Z, f32 R, f32 G, f32 B, f32 A)
{
    vertex vert;
    vert.x = X;
    vert.y = Y;
    vert.z = Z;
    vert.r = R;
    vert.g = G;
    vert.b = B;
    vert.a = A;
    return vert;
};

vertex TEXVERTEX(f32 X, f32 Y, f32 Z, f32 U, f32 V, f32 NX = 0.f, f32 NY = 0.f, f32 NZ = 0.f)
{
    vertex vert;
    vert.x = X;
    vert.y = Y;
    vert.z = Z;
    vert.u = U;
    vert.v = V;
    vert.nx = NX;
    vert.ny = NY;
    vert.nz = NZ;
    return vert;
};

enum projection_type
{
    PERSPECTIVE,
    ORTHOGRAPHIC,
};

static matrix4x4 GetViewMatrix(projection_type ProjType, v3 Location, v3 TargetPosition = V3(0, 0, 0), float PitchRotation = 0.f, float YawRotation = 0.f)
{
    DirectX::XMVECTOR defaultForward; // should never change
    DirectX::XMVECTOR camUp;
    DirectX::XMVECTOR camPosition;
    DirectX::XMVECTOR camTarget;
    DirectX::XMMATRIX CameraView;

    if (ProjType == projection_type::PERSPECTIVE)
    {
        defaultForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f );
        camUp = DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
        camPosition = DirectX::XMVectorSet(Location.x, Location.y, Location.z, 0.0f );
        camTarget = DirectX::XMVectorSet(TargetPosition.x, TargetPosition.y, TargetPosition.z, 1.0f );

        // Rotation Matrix
        DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(PitchRotation), DirectX::XMConvertToRadians(YawRotation), 0.0f);
        //DirectX::XMVECTOR camTarget = DirectX::XMVector3TransformCoord(defaultForward, RotationMatrix);
        camTarget = DirectX::XMVector3Normalize(camTarget);

        DirectX::XMMATRIX RotateYTempMatrix;
        RotateYTempMatrix = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(YawRotation));

        DirectX::XMMATRIX RotateXTempMatrix;
        RotateXTempMatrix = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(PitchRotation));
        
        camUp = XMVector3TransformCoord(XMVector3TransformCoord(camUp, RotateYTempMatrix), RotateXTempMatrix);

        camTarget = DirectX::XMVectorAdd(camTarget, camPosition);

        CameraView = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH( camPosition, camTarget, camUp ));
    }
    else
    {
        camUp = DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
        camPosition = DirectX::XMVectorSet(Location.x, Location.y, Location.z, 0.0f );
        camTarget = DirectX::XMVectorSet(TargetPosition.x, TargetPosition.y, TargetPosition.z, 0.0f );

        CameraView = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH( camPosition, camTarget, camUp ));
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

static matrix4x4 GetOrthographicProjection(int Width, int Height, float NearZ, float FarZ)
{
    matrix4x4 Result;
    
    DirectX::XMMATRIX Projection = DirectX::XMMatrixOrthographicLH((FLOAT)Width, (FLOAT)Height, NearZ, FarZ);
    Projection = DirectX::XMMatrixTranspose(Projection);

    DirectX::XMFLOAT4X4 Temp;
    DirectX::XMStoreFloat4x4(&Temp, Projection);

    Result.row1 = V4(Temp._11, Temp._12, Temp._13, Temp._14);
    Result.row2 = V4(Temp._21, Temp._22, Temp._23, Temp._24);
    Result.row3 = V4(Temp._31, Temp._32, Temp._33, Temp._34);
    Result.row4 = V4(Temp._41, Temp._42, Temp._43, Temp._44);

    return Result;
}

static matrix4x4 GetOrthographicProjectionBounded(float MinX, float MaxX, float MinY, float MaxY, float NearZ, float FarZ)
{
    matrix4x4 Result;
    
    DirectX::XMMATRIX Projection = DirectX::XMMatrixOrthographicOffCenterLH(MinX, MaxX, MinY, MaxY, NearZ, FarZ);
    Projection = DirectX::XMMatrixTranspose(Projection);

    DirectX::XMFLOAT4X4 Temp;
    DirectX::XMStoreFloat4x4(&Temp, Projection);

    Result.row1 = V4(Temp._11, Temp._12, Temp._13, Temp._14);
    Result.row2 = V4(Temp._21, Temp._22, Temp._23, Temp._24);
    Result.row3 = V4(Temp._31, Temp._32, Temp._33, Temp._34);
    Result.row4 = V4(Temp._41, Temp._42, Temp._43, Temp._44);

    return Result;
}

#define DX_H
#endif