#if !defined(LIGHTING_H)

class directional_light : public actor
{
    v4 LightColor = V4(1.f, 1.f, 1.f, 1.f);
    f32 Intensity;
};

#define LIGHTING_H
#endif