#if !defined(LEVEL_H)

class level
{

public:
    cArray<actor> ActorRegistry = cArray<actor>(MainManager, memory_lifetime::Permanent);
    virtual void Initialize() = 0;
    b2World* B2DWorld;
    void TickPhysics(float Delta);
    f32 timeStep = 1.0f / 60.0f;
    s32 velocityIterations = 6;
    s32 positionIterations = 2;
    bool Initialized = false;

protected:
    void InitializePhysics(b2Vec2 Gravity);

};

#define LEVEL_H
#endif