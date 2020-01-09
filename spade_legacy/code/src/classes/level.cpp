
#include "level.h"

void level::InitializePhysics(b2Vec2 Gravity)
{
    B2DWorld = new b2World(Gravity); // change?
    Initialized = true;
}

void level::TickPhysics(float Delta)
{
    if (Initialized)
    {
        B2DWorld->Step(timeStep, velocityIterations, positionIterations);
    }
}