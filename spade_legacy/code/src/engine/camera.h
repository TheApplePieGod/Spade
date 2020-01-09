#if !defined(CAMERA_H)

class camera
{
  private:
    f32 DegreesToRadians = Pi32 / 180.0f;

  public:
    
    v3 Position;
    
    v3 ForwardVelocity;
    v3 SideVelocity;

    v3 Velocity;

    f32 SpeedForward;
    f32 SpeedSide;

    u32 Counter = 0;

    v4 Target;
    v4 UpVector;
    v3 RightVector;
    v3 ForwardVector;

    b32 MouseMovement = false;
    bool AllowMouseMovement = true;

    f32 Width;
    f32 Height;
    f32 NearPlane;
    f32 FarPlane;

    f32 FOVDegrees;

    f32 TargetAngleInDegreesYaw;
    f32 TargetAngleInDegreesPitch;

    matrix4x4 PerspectiveProjection;
    matrix4x4 OrthographicProjection;
    matrix4x4 ViewMatrix;

    matrix3x3 ModelTransform;
    matrix3x3 WorldTransform;

    projection_type ViewMode = projection_type::PERSPECTIVE;

    camera(int X, int Y);
    void SetViewMatrix(projection_type ProjectionType, bool OrthoUseMovement = false);
    void Update(v2 MousePosDeltaXY, b32 MouseMovement);
    matrix4x4 GetPerspectiveProjectionLH(bool Transpose = true); // left hand
    matrix4x4 GetOrthographicProjectionLH(bool Transpose = true); // left hand
    matrix4x4 GenerateViewMatrix(projection_type ProjType, bool Transpose = true, bool OrthoUseMovement = false); // left hand
    DirectX::XMMATRIX GenerateWorldMatrix();

    inline matrix4x4 camera::GetProjectionMatrix()
    {
        switch(ViewMode)
        {
            case PERSPECTIVE:
            {
                return PerspectiveProjection;
            }
            case ORTHOGRAPHIC:
            {
                return OrthographicProjection;
            }
        }
        return PerspectiveProjection;
    }
};

#define CAMERA_H
#endif