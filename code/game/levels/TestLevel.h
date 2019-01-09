#if !defined(TESTLEVEL_H)

class level_TestLevel : public level
{

public:
    void Initialize() override
    {
        renderingComponent TestComp = renderingComponent();
        v3 pos = {-10, -5, -10};
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
        ActorRegistry[1].RenderingComponents.Add(TestComp);

        pos = {15, -5, 0};
        ActorRegistry.Add(actor(this), true);
        ActorRegistry[2].WorldLocation += pos;
        ActorRegistry[2].RenderingComponents.Add(TestComp);
        ActorRegistry[2].Scale = V3(0.5, 10, 15);

        pos = {0, -5, 0};
        ActorRegistry.Add(actor(this), true);
        ActorRegistry[3].WorldLocation += pos;
        ActorRegistry[3].RenderingComponents.Add(TestComp);
        ActorRegistry[3].Scale = V3(0.5, 10, 15);

        pos = {0, -5, 15};
        ActorRegistry.Add(actor(this), true);
        ActorRegistry[4].WorldLocation += pos;
        ActorRegistry[4].RenderingComponents.Add(TestComp);
        ActorRegistry[4].Scale = V3(15, 10, 0.5);

        pos = {0, -5, 0};
        ActorRegistry.Add(actor(this), true);
        ActorRegistry[5].WorldLocation += pos;
        ActorRegistry[5].RenderingComponents.Add(TestComp);
        ActorRegistry[5].Scale = V3(15, 10, 0.5);

        // ActorRegistry.Add(actor(this), true);
        // ActorRegistry[7].RenderingComponents.Add(TestComp);
        // ActorRegistry[7].Scale = V3(3, 3, 0.04);
        // ActorRegistry[7].Rotation.x = -90;
        // ActorRegistry[7].RenderingComponents[0].RenderResources.Vertices = cArray<vertex>(MainManager, memory_lifetime::Permanent);
        // ActorRegistry[7].RenderingComponents[0].RenderResources.Vertices.Append((vertex*)GlobalAssetRegistry[1]->Data, ((cMeshAsset*)GlobalAssetRegistry[2])->VertexCount, true);
    }
};

#define TESTLEVEL_H
#endif