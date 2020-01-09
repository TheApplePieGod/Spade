#if !defined(ACTOR_H)

class level;

enum actor_flag
{
    Ready,
    PendingDestroy,
};

struct physics_properties
{
    bool EnablePhysics = false;
    b2BodyType PhysicsType = b2_staticBody;
    v2 CollisionBoxExtent = V2(1.f, 1.f);
    f32 Density = 1.f;
    f32 Friction = 0.3f;
};

class actor
{

public:

    actor(level* Level)
    {
        CurrentLevel = Level;
    }

    v3 WorldLocation = V3(0, 0, 0);
    rotator Rotation = ROTATOR(0, 0, 0);
    v3 Scale = V3(1, 1, 1);

    level* CurrentLevel;

    bool Visible = true;

    cArray<renderingComponent> RenderingComponents;

    actor_flag Flag = actor_flag::Ready;
    physics_properties PhysicsProperties = physics_properties();

    // called every tick (duh)
    virtual void Tick();
    void UpdatePhysicsSettings();

    //b2d stuff
    b2Body* B2DPhysicsBody = nullptr;

private:

};

#define ACTOR_H
#endif