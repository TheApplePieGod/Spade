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
};

class widget_component;
class user_widget
{
public:
    // don't add components directly
    cArray<widget_component*> CanvasWidgets;
    cArray<widget_component*> RenderingComponents; // ease of access
    void AddChildComponent(widget_type Type, widget_component_properties DefaultProperties = widget_component_properties());
    void AddChildComponent(widget_component Component);
    v2 Position = V2((f32)-UserScreenSizeX/2, (f32)UserScreenSizeY/2);
    v2 BoundingSize = V2((f32)UserScreenSizeX, (f32)UserScreenSizeY);
    //s32 ZOrder = 0; // add support later

private:

};

class widget_component // update memory usage for carray + support for component children
{
public:
    v2 Position = V2(0, 0);
    v2 BoundingSize = V2(10, 10);
    widget_component_properties Properties;
    //inline widget_component& GetChildComponent(s32 Index);
    //inline s32 AddChildComponent(widget_component Component);
    //inline s32 GetChildCount();
    inline bool ShouldRender();
    inline widget_type GetType();
    inline s32 GetZOrder();
    void UpdateZOrder(s32 NewZ, bool AlreayInArray = true);
    s32 IndexInRenderingArray = -1; // do not change
    user_widget* Parent; // do not change
    v2 Scale = V2(1.f, 1.f);
    f32 Rotation = 0.f;

    widget_component()
    {}
    widget_component(user_widget* parent)
    {
        Parent = parent;
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
    u32 MaxChildren = 0;
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
    }
    widget_component_text(user_widget* parent)
    {
        Parent = parent;
        bShouldRender = true;
        Type = widget_type::Text;
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
    }
    widget_component_border(user_widget* parent)
    {
        Parent = parent;
        bShouldRender = true;
        Type = widget_type::Border;
    }

    void UpdateBorderImage(char ImageName[]);

    v4 Tint = colors::White;
    v2 Extent = V2(10, 10);
    std::string ImageName;
    bool UseImageBackground = false;
};

#define WIDGET_H
#endif