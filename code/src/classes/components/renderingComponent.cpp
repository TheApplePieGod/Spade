
#include "renderingComponent.h"

renderingComponent::renderingComponent()
{
    RenderResources.ShaderID = 0; // default

    vertex FrontBottomLeft =  TEXVERTEX(0.0f,0.0f,0.0f,0.0f,1.0f,0.f,0.f,-1.f);
    vertex FrontTopLeft =     TEXVERTEX(0.0f,1.0f,0.0f,0.0f,0.0f,0.f,0.f,-1.f);
    vertex FrontTopRight =    TEXVERTEX(1.0f,1.0f,0.0f,1.0f,0.0f,0.f,0.f,-1.f);
    vertex FrontBottomRight = TEXVERTEX(1.0f,0.0f,0.0f,1.0f,1.0f,0.f,0.f,-1.f);

    // Back
    vertex BackBottomLeft =  TEXVERTEX(0.0f,0.0f,1.0f,0.0f,1.0f,0.f,0.f,1.f);
    vertex BackTopLeft =     TEXVERTEX(0.0f,1.0f,1.0f,0.0f,0.0f,0.f,0.f,1.f);
    vertex BackTopRight =    TEXVERTEX(1.0f,1.0f,1.0f,1.0f,0.0f,0.f,0.f,1.f);
    vertex BackBottomRight = TEXVERTEX(1.0f,0.0f,1.0f,1.0f,1.0f,0.f,0.f,1.f);

    // Top
    vertex TopBottomLeft =      TEXVERTEX(0.0f,1.0f,0.0f,0.0f,1.0f,0.f,1.f,0.f);
    vertex TopTopLeft =         TEXVERTEX(0.0f,1.0f,1.0f,0.0f,0.0f,0.f,1.f,0.f);
    vertex TopTopRight =        TEXVERTEX(1.0f,1.0f,1.0f,1.0f,0.0f,0.f,1.f,0.f);
    vertex TopBottomRight =     TEXVERTEX(1.0f,1.0f,0.0f,1.0f,1.0f,0.f,1.f,0.f);

    // Bottom
    vertex BottomTopLeft =        TEXVERTEX(0.0f,0.0f,0.0f,0.0f,0.0f,0.f,-1.f,0.f);
    vertex BottomBottomLeft =     TEXVERTEX(0.0f,0.0f,1.0f,0.0f,1.0f,0.f,-1.f,0.f);
    vertex BottomBottomRight =    TEXVERTEX(1.0f,0.0f,1.0f,1.0f,1.0f,0.f,-1.f,0.f);
    vertex BottomTopRight =       TEXVERTEX(1.0f,0.0f,0.0f,1.0f,0.0f,0.f,-1.f,0.f);

    // Left Side
    vertex LeftBottomLeft =  TEXVERTEX(0.0f,0.0f,1.0f,0.0f,1.0f,-1.f,0.f,0.f);
    vertex LeftTopLeft =     TEXVERTEX(0.0f,1.0f,1.0f,0.0f,0.0f,-1.f,0.f,0.f);
    vertex LeftTopRight =    TEXVERTEX(0.0f,1.0f,0.0f,1.0f,0.0f,-1.f,0.f,0.f);
    vertex LeftBottomRight = TEXVERTEX(0.0f,0.0f,0.0f,1.0f,1.0f,-1.f,0.f,0.f);

    // Right Side
    vertex RightBottomLeft =  TEXVERTEX(1.0f,0.0f,0.0f,0.0f,1.0f,1.f,0.f,0.f);
    vertex RightTopLeft =     TEXVERTEX(1.0f,1.0f,0.0f,0.0f,0.0f,1.f,0.f,0.f);
    vertex RightTopRight =    TEXVERTEX(1.0f,1.0f,1.0f,1.0f,0.0f,1.f,0.f,0.f);
    vertex RightBottomRight = TEXVERTEX(1.0f,0.0f,1.0f,1.0f,1.0f,1.f,0.f,0.f);

    RenderResources.Vertices.Add(FrontBottomLeft);
    RenderResources.Vertices.Add(FrontTopLeft);
    RenderResources.Vertices.Add(FrontTopRight);

    RenderResources.Vertices.Add(FrontTopRight);
    RenderResources.Vertices.Add(FrontBottomRight);
    RenderResources.Vertices.Add(FrontBottomLeft);

    RenderResources.Vertices.Add(BackBottomRight);
    RenderResources.Vertices.Add(BackTopRight);
    RenderResources.Vertices.Add(BackTopLeft);

    RenderResources.Vertices.Add(BackTopLeft);
    RenderResources.Vertices.Add(BackBottomLeft);
    RenderResources.Vertices.Add(BackBottomRight);

    RenderResources.Vertices.Add(TopBottomLeft);
    RenderResources.Vertices.Add(TopTopLeft);
    RenderResources.Vertices.Add(TopTopRight);

    RenderResources.Vertices.Add(TopTopRight);
    RenderResources.Vertices.Add(TopBottomRight);
    RenderResources.Vertices.Add(TopBottomLeft);

    RenderResources.Vertices.Add(BottomBottomLeft);
    RenderResources.Vertices.Add(BottomTopLeft);
    RenderResources.Vertices.Add(BottomTopRight);

    RenderResources.Vertices.Add(BottomTopRight);
    RenderResources.Vertices.Add(BottomBottomRight);
    RenderResources.Vertices.Add(BottomBottomLeft);

    RenderResources.Vertices.Add(LeftBottomLeft);
    RenderResources.Vertices.Add(LeftTopLeft);
    RenderResources.Vertices.Add(LeftTopRight);

    RenderResources.Vertices.Add(LeftTopRight);
    RenderResources.Vertices.Add(LeftBottomRight);
    RenderResources.Vertices.Add(LeftBottomLeft);

    RenderResources.Vertices.Add(RightBottomLeft);
    RenderResources.Vertices.Add(RightTopLeft);
    RenderResources.Vertices.Add(RightTopRight);

    RenderResources.Vertices.Add(RightTopRight);
    RenderResources.Vertices.Add(RightBottomRight);
    RenderResources.Vertices.Add(RightBottomLeft);


    UpdateBoundingBox(LocalScale);
}

void renderingComponent::UpdateBoundingBox(v3 TotalScale)
{
    // calculate bounding box points
    v3 min = V3(RenderResources.Vertices[0].x, RenderResources.Vertices[0].y, RenderResources.Vertices[0].z);
    v3 max = V3(RenderResources.Vertices[0].x, RenderResources.Vertices[0].y, RenderResources.Vertices[0].z);
    for (u32 i = 1; i < RenderResources.Vertices.Num(); i++)
    {
        if ( RenderResources.Vertices[i].x * TotalScale.x < min.x ) min.x = RenderResources.Vertices[i].x * TotalScale.x;
        if ( RenderResources.Vertices[i].y * TotalScale.y < min.y ) min.y = RenderResources.Vertices[i].y * TotalScale.y;
        if ( RenderResources.Vertices[i].z * TotalScale.z < min.z ) min.z = RenderResources.Vertices[i].z * TotalScale.z;
        if ( RenderResources.Vertices[i].x * TotalScale.x > max.x ) max.x = RenderResources.Vertices[i].x * TotalScale.x;
        if ( RenderResources.Vertices[i].y * TotalScale.y > max.y ) max.y = RenderResources.Vertices[i].y * TotalScale.y;
        if ( RenderResources.Vertices[i].z * TotalScale.z > max.z ) max.z = RenderResources.Vertices[i].z * TotalScale.z;
    }
    RenderResources.BoundingBoxMin = min;
    RenderResources.BoundingBoxMax = max;
}