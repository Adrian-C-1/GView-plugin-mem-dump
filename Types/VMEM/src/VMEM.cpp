#include "GView.hpp"

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

#include "VMEM.hpp"

using namespace GView::Type::VMEM;

extern "C"
{
    PLUGIN_EXPORT bool Validate(const AppCUI::Utils::BufferView& buf, const std::string_view& extension)
    {
        return true;
    }
    PLUGIN_EXPORT TypeInterface* CreateInstance()
    {
        return new GView::Type::VMEM::VMEMFile();
    }
    PLUGIN_EXPORT bool PopulateWindow(Reference<WindowInterface> win)
    {
        using namespace GView::View::BufferViewer;

        auto vmem = win->GetObject()->GetContentType<VMEMFile>();

        Settings settings;
        settings.SetName("VMEM");

        vmem->selectionZoneInterface = win->GetSelectionZoneInterfaceFromViewerCreation(settings);
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