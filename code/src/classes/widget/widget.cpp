
#include "widget.h"


s32 user_widget::AddChildComponent(widget_type Type, widget_component_properties DefaultProperties) // todo: get rid of new
{
    s32 Index = -1;
    switch (Type)
    {
        case None:
        {
            widget_component* t = new widget_component(this);
            //u32 index = CanvasWidgets.Add(t);
            //if (t->ShouldRender())
            Index = t->UpdateZOrder(0, false);
            break;
        }
        case Text:
        {
            widget_component_text* t = new widget_component_text(this);
            Index = t->UpdateZOrder(0, false);
            break;
        }
        case Border:
        {
            widget_component_border* t = new widget_component_border(this);
            Index = t->UpdateZOrder(0, false);
            break;
        }
        case VerticalBox:
        {
            widget_component_verticalbox* t = new widget_component_verticalbox(this);
            Index = t->UpdateZOrder(0, false);
            break;
        }
        case HorizontalBox:
        {
            widget_component_horizontalbox* t = new widget_component_horizontalbox(this);
            Index = t->UpdateZOrder(0, false);
            break;
        }
        case WrapBox:
        {
            widget_component_wrapbox* t = new widget_component_wrapbox(this);
            Index = t->UpdateZOrder(0, false);
            break;
        }
    }
    return Index;
}

/*
* Searches through the widget hierarchy and returns first instance that has a matching tag
*/
widget_component* user_widget::GetComponentByTag(std::string Tag)
{
    for (u32 i = 0; i < AllComponents.Num(); i++)
    {
        if (AllComponents[i]->Tag == Tag)
        {
            return AllComponents[i];
        }
    }
    Assert(1==2);
    return nullptr;
}

/*
* Creates a copy of passed comp and adds it
*/
s32 user_widget::AddChildComponent(widget_component* Component)
{
    s32 Index = -1;
    switch (Component->GetType())
    {
        case None:
        {
            widget_component* t = new widget_component(Component, this);
            //u32 index = CanvasWidgets.Add(t);
            //if (t->ShouldRender())
            Index = t->UpdateZOrder(0, false);
            break;
        }
        case Text:
        {
            widget_component_text* t = new widget_component_text(fast_dynamic_cast<widget_component_text*>(Component), this);
            Index = t->UpdateZOrder(0, false);
            break;
        }
        case Border:
        {
            widget_component_border* t = new widget_component_border(fast_dynamic_cast<widget_component_border*>(Component), this);
            Index = t->UpdateZOrder(0, false);
            break;
        }
        case VerticalBox:
        {
            widget_component_verticalbox* t = new widget_component_verticalbox(fast_dynamic_cast<widget_component_verticalbox*>(Component), this);
            Index = t->UpdateZOrder(0, false);
            break;
        }
        case HorizontalBox:
        {
            widget_component_horizontalbox* t = new widget_component_horizontalbox(fast_dynamic_cast<widget_component_horizontalbox*>(Component), this);
            Index = t->UpdateZOrder(0, false);
            break;
        }
    }
    return Index;
}

/*
* Creates a copy of passed comp
*/
int_bool widget_component::AddChildComponent(widget_component* Component)
{
    int_bool Out;
    if (Children.Num() >= MaxChildren)
    {
        return Out;
    }
    else
    {
        switch (Component->GetType())
        {
            case None:
            {
                widget_component* t = new widget_component(Component, Parent);
                //u32 index = CanvasWidgets.Add(t);
                //if (t->ShouldRender())
                t->ParentComponent = this;
                Out.Int = t->UpdateZOrder(t->ZOrder, false); // redundant?
                break;
            }
            case Text:
            {
                widget_component_text* t = new widget_component_text(fast_dynamic_cast<widget_component_text*>(Component), Parent);
                t->ParentComponent = this;
                Out.Int = t->UpdateZOrder(t->ZOrder, false);
                break;
            }
            case Border:
            {
                widget_component_border* t = new widget_component_border(fast_dynamic_cast<widget_component_border*>(Component), Parent);
                t->ParentComponent = this;
                Out.Int = t->UpdateZOrder(t->ZOrder, false);
                break;
            }
            case VerticalBox:
            {
                widget_component_verticalbox* t = new widget_component_verticalbox(fast_dynamic_cast<widget_component_verticalbox*>(Component), Parent);
                t->ParentComponent = this;
                Out.Int = t->UpdateZOrder(t->ZOrder, false);
                break;
            }
            case HorizontalBox:
            {
                widget_component_horizontalbox* t = new widget_component_horizontalbox(fast_dynamic_cast<widget_component_horizontalbox*>(Component), Parent);
                t->ParentComponent = this;
                Out.Int = t->UpdateZOrder(t->ZOrder, false);
                break;
            }
        }
        Out.Bool = true;
        return Out;
    }
}

/*
* Copies user_widget into widget_component (slow)
*/
// s32 user_widget::AddChildComponent(user_widget* Component)
// {
//     // 'none' type
//     widget_component* NewComp = new widget_component(this);
//     NewComp->Children.Append(Component->RenderingComponents);
//     for (u32 i = 0; i < Component->RenderingComponents.Num(); i++)
//     {
//         NewComp->Children.Add();
//     }
//     NewComp->Position = Component->Position;
//     return NewComp->UpdateZOrder(0, false);
// }

/*
* Copies user_widget into widget_component (slow)
*/
// int_bool widget_component::AddChildComponent(user_widget* Component)
// {
//     int_bool Out;
//     if (Children.Num() >= MaxChildren)
//     {
//         return Out;
//     }
//     else
//     {
//         widget_component* NewComp = new widget_component(Parent);
//         NewComp->ParentComponent = this;
//         NewComp->Children.Append(Component->RenderingComponents);
//         for (u32 i = 0; i < NewComp->Children.Num(); i++)
//         {
//             NewComp->Children[i]->ParentComponent = NewComp;
//         }
//         NewComp->Position = Component->Position;
//         Out.Int = NewComp->UpdateZOrder(0, false);
//         Out.Bool = true;
//         return Out;
//     }
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

/*
* Returns new index inside array
*/
s32 widget_component::UpdateZOrder(s32 NewZ, bool AlreadyInArray)
{
    ZOrder = NewZ;
    if (ParentComponent == nullptr) // if comp is direct child of widget
    {
        if (AlreadyInArray)
            Parent->RenderingComponents.RemoveAt(IndexInRenderingArray);
        for (u32 i = 0; i < Parent->RenderingComponents.Num(); i++)
        {
            if (Parent->RenderingComponents[i]->GetZOrder() > NewZ)
            {
                Parent->RenderingComponents.AddAt(i, this);
                HierarchyIndex = Parent->AllComponents.Add(this);
                IndexInRenderingArray = i;
                return i;
            }
        }
        IndexInRenderingArray = Parent->RenderingComponents.Num();
        Parent->RenderingComponents.Add(this);
        HierarchyIndex = Parent->AllComponents.Add(this);
    }
    else
    {
        if (AlreadyInArray)
            ParentComponent->Children.RemoveAt(IndexInRenderingArray);
        for (u32 i = 0; i < ParentComponent->Children.Num(); i++)
        {
            if (ParentComponent->Children[i]->GetZOrder() > NewZ)
            {
                ParentComponent->Children.AddAt(i, this);
                HierarchyIndex = ParentComponent->Parent->AllComponents.Add(this);
                IndexInRenderingArray = i;
                return i;
            }
        }
        IndexInRenderingArray = ParentComponent->Children.Num();
        ParentComponent->Children.Add(this);
        HierarchyIndex = ParentComponent->Parent->AllComponents.Add(this);
    }
    return IndexInRenderingArray;
}

void widget_component_border::UpdateBorderImage(char NewName[])
{
    char Buffer[MAX_PATH];
    _snprintf_s(Buffer, sizeof(Buffer), "%s", NewName);
    ImageName.erase();
    ImageName.append(Buffer, MAX_PATH);
}