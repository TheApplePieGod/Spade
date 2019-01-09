
#include "camera.h"

camera::camera(int X, int Y) : Width((f32)X), Height((f32)Y)
{
    Width;
    Height;
    FOVDegrees = 70.0f;
    NearPlane = 0.01f;
    FarPlane = 600.0f;

    // SpeedForward = 0.001;
    // SpeedSide = 0.001;
    SpeedForward = 0.05;
    SpeedSide = 0.05;

    Position.x = 0.0f;
    Position.y = 0.0f;
    Position.z = -1.0f;
    Velocity.x = 0.0f;
    Velocity.y = 0.0f;
    Velocity.z = 0.0f;
    Target.x = 0.0f;
    Target.y = 0.0f;
    Target.z = 1.0f;
    Target.w = 0.0f;
    UpVector.x = 0.0f;
    UpVector.y = 1.0f;
    UpVector.z = 0.0f;
    UpVector.w = 0.0f;
    ForwardVector.x = 0.0f;
    ForwardVector.y = 0.0f;
    ForwardVector.z = 1.0f;
    ForwardVector.w = 0.0f;

    TargetAngleInDegreesYaw = 0.0f;
    TargetAngleInDegreesPitch = 0.0f;

    // TODO: Move this into the camera
    PerspectiveProjection = GetPerspectiveProjectionLH(true);

    OrthographicProjection = GetOrthographicProjectionLH(true);

    ViewMatrix = GenerateViewMatrix(projection_type::PERSPECTIVE);
}

matrix4x4 camera::GetPerspectiveProjectionLH(bool Transpose)
{
    matrix4x4 Result;
    
    DirectX::XMMATRIX Projection = DirectX::XMMatrixPerspectiveFovLH(FOVDegrees * (Pi32 / 180.0f), Width / Height, NearPlane, FarPlane);
    if (Transpose)
    {
        Projection = DirectX::XMMatrixTranspose(Projection);
    }

    DirectX::XMFLOAT4X4 Temp;
    DirectX::XMStoreFloat4x4(&Temp, Projection);

    Result.row1 = V4(Temp._11, Temp._12, Temp._13, Temp._14);
    Result.row2 = V4(Temp._21, Temp._22, Temp._23, Temp._24);
    Result.row3 = V4(Temp._31, Temp._32, Temp._33, Temp._34);
    Result.row4 = V4(Temp._41, Temp._42, Temp._43, Temp._44);    

    return Result;
}

matrix4x4 camera::GetOrthographicProjectionLH(bool Transpose)
{
    matrix4x4 Result;
    
    DirectX::XMMATRIX Projection = DirectX::XMMatrixOrthographicLH(Width, Height, NearPlane, FarPlane);
    if (Transpose)
    {
        Projection = DirectX::XMMatrixTranspose(Projection);
    }

    DirectX::XMFLOAT4X4 Temp;
    DirectX::XMStoreFloat4x4(&Temp, Projection);

    Result.row1 = V4(Temp._11, Temp._12, Temp._13, Temp._14);
    Result.row2 = V4(Temp._21, Temp._22, Temp._23, Temp._24);
    Result.row3 = V4(Temp._31, Temp._32, Temp._33, Temp._34);
    Result.row4 = V4(Temp._41, Temp._42, Temp._43, Temp._44);

    return Result;
}

matrix4x4 camera::GenerateViewMatrix(projection_type ProjType, bool Transpose, bool OrthoUseMovement)
{
    DirectX::XMVECTOR defaultForward;
    DirectX::XMVECTOR camUp;
    DirectX::XMVECTOR camPosition;
    DirectX::XMVECTOR camTarget;
    DirectX::XMMATRIX CameraView;

    if (ProjType == projection_type::PERSPECTIVE)
    {

        f32 DebugCameraOffset = -0.0f;


        defaultForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f );
        camUp = DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
        camPosition = DirectX::XMVectorSet(Position.x, Position.y, Position.z, 0.0f );

        // Rotation Matrix
        DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(TargetAngleInDegreesPitch), DirectX::XMConvertToRadians(TargetAngleInDegreesYaw), 0.0f);
        DirectX::XMVECTOR camTarget = DirectX::XMVector3TransformCoord(defaultForward, RotationMatrix);
        camTarget = DirectX::XMVector3Normalize(camTarget);
        
        DirectX::XMFLOAT4 temp;    //the float where we copy the v2 vector members
        DirectX::XMStoreFloat4(&temp, camTarget);   //the function used to copy
        Target.x = temp.x;
        Target.y = temp.y;
        Target.z = temp.z;

        DirectX::XMMATRIX RotateYTempMatrix;
        RotateYTempMatrix = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(TargetAngleInDegreesYaw));

        DirectX::XMMATRIX RotateXTempMatrix;
        RotateXTempMatrix = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(TargetAngleInDegreesPitch));
        
        camUp = XMVector3TransformCoord(XMVector3TransformCoord(camUp, RotateYTempMatrix), RotateXTempMatrix);

        camTarget = DirectX::XMVectorAdd(camTarget, camPosition);

        if (Transpose)
            CameraView = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH( camPosition, camTarget, camUp ));
        else
            CameraView = DirectX::XMMatrixLookAtLH( camPosition, camTarget, camUp );

    }
    else
    {
        camUp = DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
        if (OrthoUseMovement)
            camPosition = DirectX::XMVectorSet(Position.x, Position.y, Position.z, 0.0f );
        else
            camPosition = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f );
        camTarget = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f );

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

void camera::SetViewMatrix(projection_type ProjectionType, bool OrthoUseMovement)
{
    ViewMatrix = GenerateViewMatrix(ProjectionType, true, OrthoUseMovement);
}

void camera::Update(v2 MousePosDeltaXY, b32 MouseMovement)
{
    ForwardVector = V3Normalize(V3(Target.x, Target.y, Target.z));
    RightVector = V3CrossProduct(V3(UpVector.x, UpVector.y, UpVector.z), ForwardVector);
    if (MouseMovement && AllowMouseMovement)
    {
        TargetAngleInDegreesYaw += (MousePosDeltaXY.x * 0.1f);
        TargetAngleInDegreesPitch += (MousePosDeltaXY.y * 0.1f);
    }
}

DirectX::XMMATRIX camera::GenerateWorldMatrix()
{
    DirectX::XMMATRIX translation, rotationx, rotationy, rotationz;
    translation = DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z);
    rotationx = DirectX::XMMatrixRotationX(0);
    rotationy = DirectX::XMMatrixRotationY(TargetAngleInDegreesPitch * (Pi32 / 180.0f));
    rotationz = DirectX::XMMatrixRotationZ(TargetAngleInDegreesYaw * (Pi32 / 180.0f));

    return rotationx * rotationy * rotationz * translation;
}