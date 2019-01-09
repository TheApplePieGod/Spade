#if !defined(MATERIAL_H)

class material
{

public:

    v3 Normal = V3(0, 0, 0);

    v4 EmissiveColor = V4(0.f, 0.f, 0.f, 0.f);
    // loaded and initialized texture shader ref
    ID3D11ShaderResourceView* DiffuseTexture = nullptr;
    v4 DiffuseColor = V4(1.f, 1.f, 1.f, 1.f);
    b32 bUsesTextureDiffuse = false;

    f32 SpecularPower = 1;

private:

};

#define MATERIAL_H
#endif