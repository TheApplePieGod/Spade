#if !defined(RENDERINGCOMPONENT_H)

enum rendering_specials
{
    Default,
    Tilemap,
};

class renderingComponent
{

public:

    mesh_render_resources RenderResources;
    rendering_specials SpecialType = rendering_specials::Default;

    v3 LocalLocation = V3(0, 0, 0);
    v3 LocalScale = V3(1, 1, 1);
    rotator LocalRotation = ROTATOR(0, 0, 0);

    bool Visible = true;
    bool CastShadow = true;

    renderingComponent();
    void UpdateBoundingBox(v3 TotalScale);

private:

protected:


};

#define RENDERINGCOMPONENT_H
#endif

