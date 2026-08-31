#include "VMEM.hpp"

#include "VMEM.hpp"

using namespace GView::Type::VMEM;
using namespace AppCUI::Controls;

Panels::Debug::Debug(Reference<VMEMFile> _vmem) 
    : TabPage("Debugger") 
{
    vmem = _vmem;
    
    refresh = Factory::Button::Create(this, "Refresh", "x:0,y:1,w:100%,h:2");

    debug = Factory::Label::Create(this, "", "x:0,y:5,w:100%,h:100%");
    
    debug->SetText("Debug text here");
}

void Panels::Debug::OnAfterResize(int nw, int nh){
    ;
}

bool Panels::Debug::OnEvent(Reference<Control> sender, Event evnt, int controlID)
{
    // vector<char> v = vmem->dumpAnalyzer->latestDebug;
    // memset(v.data() + strlen(v.data()), 0, v.max_size() - strlen(v.data()) - 2);
    // da segfault
    
    debug->SetText(vmem->dumpAnalyzer->latestDebug.data(), false);
    return false;
}

void Panels::Debug::Paint(AppCUI::Graphics::Renderer& renderer)
{
    ;
    TabPage::Paint(renderer);
}
