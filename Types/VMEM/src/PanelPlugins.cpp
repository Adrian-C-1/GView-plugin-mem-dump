#include "VMEM.hpp"

using namespace AppCUI;

using namespace GView::Type::VMEM;
using namespace AppCUI::Controls;

constexpr int CMD_BUTTON_SUBMIT = 1;

constexpr int CMD_VIRTUAL_MEMORY_ANALYZER = 2;

Panels::Plugins::Plugins(Reference<VMEMFile> _vmem) 
    : TabPage("Dump File Plugins")
{
    vmem = _vmem;

    general = Factory::ListView::Create(
        this, "x:0,y:0,w:100%,h:50%", 
        { "n:Plugin Name,w:20", "n:Description,w:100" }, 
        ListViewFlags::None
    );
    general->AddItem({ "Virtual Memory Analyzer", "Analyzes the virtual memory dump and provides insights" }).SetData(CMD_VIRTUAL_MEMORY_ANALYZER);
    general->AddItem({ "Plugin 1", "Description of Plugin 1" });
    general->AddItem({ "Plugin 2", "Description of Plugin 2" });
    general->AddItem({ "Plugin 3", "Description of Plugin 3" });

    inputLabel = Factory::Button::Create(this, "Input:", "x:0,y:50%,w:7,h:1");
    input = Factory::TextField::Create(this, "0xffff0000", "x:6,y:50%,w:100,h:1");
    
    submit = Factory::Button::Create(this, "Submit", "x:0,y:60%,w:100%,h:1", CMD_BUTTON_SUBMIT);

    debug = Factory::Label::Create(this, "Debug aici :3", "x:0,y:99%,w:100%,h:1");
}

bool Panels::Plugins::OnEvent(Reference<Control> sender, Event evnt, int controlID)
{
    if (TabPage::OnEvent(sender, evnt, controlID)){
        return true;
    }

    if ((evnt == Event::ButtonClicked) && (controlID == CMD_BUTTON_SUBMIT))
    {
        std::string a;
        auto text = input->GetText().ToString(a);
        this->debug->SetText(LocalString<128>().Format("Stringul este %s", a.c_str()));
        return true;
    }
    if ((evnt == Event::ListViewCurrentItemChanged) && (sender == general.ToBase<Control>()))
    {
        auto item = general->GetCurrentItem();
        int idx = item.GetData(0);
        if (idx == CMD_VIRTUAL_MEMORY_ANALYZER){
            ;
            // psloadedmodulelist 0xfffff80220013470
        }
        return true;
    }

    return false;
}

void Panels::Plugins::Update()
{
    
}

void Panels::Plugins::Paint(AppCUI::Graphics::Renderer& renderer)
{
    
    TabPage::Paint(renderer);
}

void Panels::Plugins::OnAfterResize(int newWidth, int newHeight)
{
    TabPage::OnAfterResize(newWidth, newHeight);
}