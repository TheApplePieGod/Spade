
void actor::Tick()
{

}

/*
* Set members in PhysicsProperties before calling
*/
void actor::UpdatePhysicsSettings()
{
    if (B2DPhysicsBody != nullptr)
        CurrentLevel->B2DWorld->DestroyBody(B2DPhysicsBody);
    if (PhysicsProperties.EnablePhysics)
    {
        switch(PhysicsProperties.PhysicsType)
        {
            case b2_kinematicBody: // not supported
            case b2_staticBody:
            {
                b2BodyDef bodyDef;
                bodyDef.type = b2_staticBody;
                bodyDef.position.Set(WorldLocation.x, WorldLocation.y);
                B2DPhysicsBody = CurrentLevel->B2DWorld->CreateBody(&bodyDef);
                break;
            }
            case b2_dynamicBody:
            {
                b2BodyDef bodyDef;
                bodyDef.type = b2_dynamicBody;
                bodyDef.position.Set(WorldLocation.x, WorldLocation.y);
                B2DPhysicsBody = CurrentLevel->B2DWorld->CreateBody(&bodyDef);
                break;
            }
        }

        b2PolygonShape dynamicBox;
        dynamicBox.SetAsBox(PhysicsProperties.CollisionBoxExtent.x / 2, PhysicsProperties.CollisionBoxExtent.y / 2);
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = PhysicsProperties.Density;
        fixtureDef.friction = PhysicsProperties.Friction;

        B2DPhysicsBody->CreateFixture(&fixtureDef);
    }
}