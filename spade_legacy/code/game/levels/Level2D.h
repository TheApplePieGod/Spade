#if !defined(LEVEL2D_H)

class level_Level2D : public level
{

public:
    b2Body* Bodd;
    void Initialize() override
    {
        b2Vec2 Gravity(0.0f, -0.1f);
        InitializePhysics(Gravity);
        
        renderingComponent TestComp = renderingComponent();
        v3 pos = {-10.f, -2.f, 0};
        for (int i = 0; i < 1; i++) // stress test
        {
            TestComp.RenderResources.ShaderID = GetShaderIDFromName("arate.png");//(rand() % 4) + 1;
            ActorRegistry.Add(actor(this), true);
            ActorRegistry[i].WorldLocation += pos;
            ActorRegistry[i].RenderingComponents.Add(TestComp);
            ActorRegistry[i].Scale = ActorRegistry[i].Scale + V3(50, 0, 50);
            //pos += V3(0, 3.25, 0);
            pos = V3((f32)(rand() % 500), (f32)(rand() % 500), (f32)(rand() % 500));
        }

        pos = {0, 5, 0};
        ActorRegistry.Add(actor(this), true);
        ActorRegistry[1].WorldLocation += pos;
        ActorRegistry[1].Scale = V3(1.f,1.f,1.f);
        ActorRegistry[1].RenderingComponents.Add(TestComp);
        physics_properties Props;
        Props.EnablePhysics = true;
        Props.PhysicsType = b2_dynamicBody;
        ActorRegistry[1].PhysicsProperties = Props;
        ActorRegistry[1].UpdatePhysicsSettings();

        b2BodyDef GroundBodyDef;
        GroundBodyDef.position.Set(-10.f, -2.f);
        b2Body* GroundBody = B2DWorld->CreateBody(&GroundBodyDef);

        b2PolygonShape GroundBox;
        GroundBox.SetAsBox(100.0f, 0.5f);
        GroundBody->CreateFixture(&GroundBox, 0.0f);

        //TODO : CHANGE FILTERING WITH TILEMAPS
        u32 ID = GetShaderIDFromName("Tilemap1.png");
        tileset NewSet = tileset(GlobalTextureRegistry[ID], ID, 32, 32);

        tilemap RenderMap = tilemap(NewSet);
        RenderMap.InitializeMap();

        pos = {0, 10, 0};
        ActorRegistry.Add(actor(this), true);
        ActorRegistry[2].WorldLocation += pos;
        ActorRegistry[2].Scale = V3(1.f,1.f,1.f);
        ActorRegistry[2].RenderingComponents.Add(RenderMap);

        // pos = {0, 5, 0};
        // ActorRegistry.Add(actor(this), true);
        // ActorRegistry[3].WorldLocation += pos;
        // ActorRegistry[3].Scale = V3(1.f,1.f,1.f);
        // ActorRegistry[3].RenderingComponents.Add(TestComp);


        // ActorRegistry[2].RenderingComponents.Add(renderingComponent());
        // ActorRegistry[2].RenderingComponents[0].RenderResources.Vertices.Clear();
        // ActorRegistry[2].RenderingComponents[0].RenderResources.Vertices.Append(RenderMap.RenderResources.Vertices, true);
        // ActorRegistry[2].RenderingComponents[0].RenderResources.ShaderID = RenderMap.RenderResources.ShaderID;

        // ActorRegistry.Add(actor(), true);
        // ActorRegistry[7].RenderingComponents.Add(TestComp);
        // ActorRegistry[7].Scale = V3(3, 3, 0.04);
        // ActorRegistry[7].Rotation.x = -90;
        // ActorRegistry[7].RenderingComponents[0].RenderResources.Vertices = cArray<vertex>(MainManager, memory_lifetime::Permanent);
        // ActorRegistry[7].RenderingComponents[0].RenderResources.Vertices.Append((vertex*)GlobalAssetRegistry[1]->Data, ((cMeshAsset*)GlobalAssetRegistry[2])->VertexCount, true);
    }
};

#define LEVEL2D_H
#endif