#include "VMEM.hpp"

using namespace GView::Type::VMEM;
using namespace AppCUI::Controls;

constexpr int CMD_BUTTON_BACK = 1;

Panels::Structure::Structure(Reference<VMEMFile> _vmem) 
    : TabPage("Structure Parser")
{
    vmem = _vmem;

    back = Factory::Button::Create(this, "&Back", "x:0,y:0,w:20", CMD_BUTTON_BACK);
}

bool Panels::Structure::OnEvent(Reference<Control> sender, Event evnt, int controlID)
{
    if (TabPage::OnEvent(sender, evnt, controlID)){
        return true;
    }

    if ((evnt == Event::ButtonClicked) && (controlID == CMD_BUTTON_BACK))
    {
        auto tab = this->GetParent().ToObjectRef<Tab>();
        tab->SetCurrentTabPageByRef(this->GetParent().ToObjectRef<Tab>()->GetChild(0), true);
        return true;
    }

    return false;
}

void Panels::Structure::Update()
{
    
}

void Panels::Structure::Paint(AppCUI::Graphics::Renderer& renderer)
{
    
    TabPage::Paint(renderer);
}

void Panels::Structure::OnAfterResize(int newWidth, int newHeight)
{
    TabPage::OnAfterResize(newWidth, newHeight);
}