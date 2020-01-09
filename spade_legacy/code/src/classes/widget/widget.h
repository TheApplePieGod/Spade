#if !defined(WIDGET_H)

struct widget_component_properties
{
    bool Visible = true;
};

enum widget_type
{
    None,
    Text,
    Border,
    VerticalBox,
    HorizontalBox,
    WrapBox,
};

class widget_component;
class user_widget
{
public:
    // don't add components directly, but returned indexes reference this array
    cArray<widget_component*> RenderingComponents;
    s32 AddChildComponent(widget_type Type, widget_component_properties DefaultProperties = widget_component_properties());
    s32 AddChildComponent(widget_component* Component);
    s32 AddChildComponent(user_widget* Component);
    widget_component* GetComponentByTag(std::string Tag);
    v2 Position = V2((f32)-UserScreenSizeX/2, (f32)UserScreenSizeY/2);
    v2 BoundingSize = V2((f32)UserScreenSizeX, (f32)UserScreenSizeY);
    //s32 ZOrder = 0; // add support later

    // all components in hierarchy. internal reference only
    cArray<widget_component*> AllComponents;

private:

};

class widget_component // update memory usage for carray + DEFAULT TAG NAMES?
{
public:
    v2 Position = V2(0, 0);
    v2 TempPosition = Position;
    v2 Extent = V2(10, 10);
    widget_component_properties Properties;
    //inline widget_component& GetChildComponent(s32 Index);
    inline int_bool AddChildComponent(widget_component* Component);
    inline int_bool AddChildComponent(user_widget* Component);
    //inline s32 GetChildCount();
    inline bool ShouldRender();
    inline widget_type GetType();
    inline s32 GetZOrder();
    s32 UpdateZOrder(s32 NewZ, bool AlreayInArray = true);
    s32 IndexInRenderingArray = -1; // do not change
    s32 HierarchyIndex = -1; // do not change. internal reference only
    std::string Tag; // way to find widgets
    widget_component* ParentComponent = nullptr;
    user_widget* Parent; // do not change
    v2 Scale = V2(1.f, 1.f);
    f32 Rotation = 0.f;
    // do not add directly
    cArray<widget_component*> Children = cArray<widget_component*>(MainManager, memory_lifetime::Permanent);

    widget_component()
    {}
    widget_component(const widget_component* A, user_widget* ParentW)
    {
        Position = A->Position;
        Extent = A->Extent;
        Properties = A->Properties;
        Parent = ParentW;
        Rotation = A->Rotation;
        Scale = A->Scale;
        MaxChildren = A->MaxChildren;
        bShouldRender = A->bShouldRender;
        ZOrder = A->ZOrder;

        // copy children
        for (u32 i = 0; i < A->Children.Num(); i++)
        {
            // switching, adding copies of user_widget
        }
    }
    widget_component(user_widget* parent)
    {
        Parent = parent;
    }

    virtual ~widget_component()
    {
        Children.ManualFree();
    }
    // template <typename OtherClassType>
    // inline bool IsA() const
    // {
    //     const OtherClassType* Check = dynamic_cast<const OtherClassType*>(this);
    //     if (Check != nullptr)
    //         return true;
    //     else
    //         return false;
    // }

protected:
    u32 MaxChildren = 999;
    //cArray<widget_component> ChildComponents;
    bool bShouldRender = false;
    widget_type Type = widget_type::None;
    s32 ZOrder = 0;

};

class widget_component_text : public widget_component
{
public:

    widget_component_text()
    {
        bShouldRender = true;
        Type = widget_type::Text;
        MaxChildren = 0;
        Children.ManualFree(); // free because no children are allowed
    }
    widget_component_text(const widget_component_text* A, user_widget* ParentW)
    {
        widget_component::widget_component(A, ParentW);
        Text = A->Text;
        Color = A->Color;
    }
    widget_component_text(user_widget* parent)
    {
        Parent = parent;
        bShouldRender = true;
        Type = widget_type::Text;
        MaxChildren = 0;
        Children.ManualFree(); // free because no children are allowed
    }

    std::string Text = "Text";
    v4 Color = colors::Black;
};

class widget_component_border : public widget_component
{
public:

    widget_component_border()
    {
        bShouldRender = true;
        Type = widget_type::Border;
        MaxChildren = 1;
    }
    widget_component_border(const widget_component_border* A, user_widget* ParentW)
    {
        widget_component::widget_component(A, ParentW);
        Tint = A->Tint;
        ImageName = A->ImageName;
        UseImageBackground = A->UseImageBackground;
    }
    widget_component_border(user_widget* parent)
    {
        Parent = parent;
        bShouldRender = true;
        Type = widget_type::Border;
        MaxChildren = 1;
    }

    void UpdateBorderImage(char ImageName[]);

    v4 Tint = colors::White;
    std::string ImageName;
    bool UseImageBackground = false;
};

class widget_component_verticalbox : public widget_component
{
public:

    widget_component_verticalbox()
    {
        bShouldRender = true;
        Type = widget_type::VerticalBox;
        MaxChildren = 999;
    }
    widget_component_verticalbox(const widget_component_verticalbox* A, user_widget* ParentW)
    {
        widget_component::widget_component(A, ParentW);
        Padding = A->Padding;
        FillDownward = A->FillDownward;
    }
    widget_component_verticalbox(user_widget* parent)
    {
        Parent = parent;
        bShouldRender = true;
        Type = widget_type::VerticalBox;
        MaxChildren = 999;
    }

    // padding between widget children
    float Padding = 5.f;
    bool FillDownward = true;
};

class widget_component_horizontalbox : public widget_component
{
public:

    widget_component_horizontalbox()
    {
        bShouldRender = true;
        Type = widget_type::HorizontalBox;
        MaxChildren = 999;
    }
    widget_component_horizontalbox(const widget_component_horizontalbox* A, user_widget* ParentW)
    {
        widget_component::widget_component(A, ParentW);
        Padding = A->Padding;
        FillRight = A->FillRight;
    }
    widget_component_horizontalbox(user_widget* parent)
    {
        Parent = parent;
        bShouldRender = true;
        Type = widget_type::HorizontalBox;
        MaxChildren = 999;
    }

    // padding between widget children
    float Padding = 5.f;
    bool FillRight = true;
};

class widget_component_wrapbox : public widget_component
{
public:

    widget_component_wrapbox()
    {
        bShouldRender = true;
        Type = widget_type::WrapBox;
        MaxChildren = 999;
    }
    widget_component_wrapbox(const widget_component_wrapbox* A, user_widget* ParentW)
    {
        widget_component::widget_component(A, ParentW);
        Padding = A->Padding;
        ExplicitWrap = A->ExplicitWrap;
    }
    widget_component_wrapbox(user_widget* parent)
    {
        Parent = parent;
        bShouldRender = true;
        Type = widget_type::WrapBox;
        MaxChildren = 999;
    }

    // padding between widget children
    float Padding = 5.f;
    float ExplicitWrap = 0.f;
};

#define WIDGET_H
#endif