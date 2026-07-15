#include "GView.hpp"
#include "VMEM.hpp"

#include <stdio.h>
#include <stdarg.h>

using namespace AppCUI;
using namespace AppCUI::Utils;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace GView::Utils;
// using namespace GView::Type;
using namespace GView;
using namespace GView::View;


using namespace GView::Type::VMEM;

extern "C"
{
    PLUGIN_EXPORT bool Validate(const AppCUI::Utils::BufferView& buf, const std::string_view& extension)
    {
        return true;
    }
    PLUGIN_EXPORT TypeInterface* CreateInstance()
    {
        return new VMEMFile();
    }
    PLUGIN_EXPORT bool PopulateWindowBackup(Reference<WindowInterface> win){
        using namespace GView::View::BufferViewer;
    
        auto vmem = win->GetObject()->GetContentType<VMEMFile>();
    
        BufferViewer::Settings settings;
        settings.SetName("VMEM");
    
        vmem->selectionZoneInterface = win->GetSelectionZoneInterfaceFromViewerCreation(settings);
    
        // win->CreateViewer(settings);
        
        // win->AddPanel();
        return true;
    }
    PLUGIN_EXPORT bool PopulateWindow(Reference<WindowInterface> win)
    {
        using namespace GView::View::BufferViewer;
    
        auto vmem = win->GetObject()->GetContentType<VMEMFile>();
    
        // asta e in viewer propriu
        win->CreateViewer<VMEMViewer::Settings>("VMEM SETTING");

        // asta imi creaza un buferviewer
        // bufferviewer se loadeaza mult mai repede dacat textview
        if (1 == 1){
            BufferViewer::Settings settings;
            settings.SetName("VMEM");
            vmem->selectionZoneInterface = win->GetSelectionZoneInterfaceFromViewerCreation(settings);
        }
        // asta imi creaza un text view (e slow asf)
        if (1 == 1){
            win->CreateViewer<TextViewer::Settings>("Text View name?");
        }

        // asta e panelu din dreapta cu informatii despre file
        auto* panel = new GView::Type::VMEM::Panels::Information(vmem);
        win->AddPanel(AppCUI::Utils::Pointer<TabPage>(panel), true);

        // win->AddPanel(AppCUI::Utils::Pointer<TabPage>(new Panels::Information(vmem)), true);
        return true;
    }
    PLUGIN_EXPORT void UpdateSettings(IniSection sect)
    {
        sect["Extension"] = { "vmem" };
        sect["Priority"]    = 1;
        sect["Description"] = "VMEM format Adrian";
    }
}

int main(){
    return 0;
}