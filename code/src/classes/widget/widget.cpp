
#include "widget.h"


void user_widget::AddChildComponent(widget_type Type, widget_component_properties DefaultProperties) // todo: get rid of new
{
    switch (Type)
    {
        case None:
        {
            return;
        }
        case Text:
        {
            widget_component_text* t = new widget_component_text(this);
            u32 index = CanvasWidgets.Add(t);
            if (t->ShouldRender())
                CanvasWidgets[index]->UpdateZOrder(0, false);
            break;
        }
        case Border:
        {
            widget_component_border* t = new widget_component_border(this);
            u32 index = CanvasWidgets.Add(t);
            if (t->ShouldRender())
                CanvasWidgets[index]->UpdateZOrder(0, false);
            break;
        }
    }
}

void user_widget::AddChildComponent(widget_component Component)
{

}

// widget_component& widget_component::GetChildComponent(s32 Index)
// {
//     return ChildComponents[Index];
// }

// s32 widget_component::AddChildComponent(widget_component Component)
// {
//     return ChildComponents.Add(Component);
// }

// s32 widget_component::GetChildCount()
// {
//     return ChildComponents.Num();
// }

bool widget_component::ShouldRender()
{
    return bShouldRender;
}

widget_type widget_component::GetType()
{
    return Type;
}

s32 widget_component::GetZOrder()
{
    return ZOrder;
}

void widget_component::UpdateZOrder(s32 NewZ, bool AlreadyInArray)
{
    ZOrder = NewZ;
    if (AlreadyInArray)
        Parent->RenderingComponents.RemoveAt(IndexInRenderingArray);
    for (u32 i = 0; i < Parent->RenderingComponents.Num(); i++)
    {
        if (Parent->RenderingComponents[i]->GetZOrder() > NewZ)
        {
            Parent->RenderingComponents.AddAt(i, this);
            IndexInRenderingArray = i;
            return;
        }
    }
    IndexInRenderingArray = Parent->RenderingComponents.Num();
    Parent->RenderingComponents.Add(this);
}

void widget_component_border::UpdateBorderImage(char NewName[])
{
    char Buffer[MAX_PATH];
    _snprintf_s(Buffer, sizeof(Buffer), "%s", NewName);
    ImageName.erase();
    ImageName.append(Buffer, MAX_PATH);
}