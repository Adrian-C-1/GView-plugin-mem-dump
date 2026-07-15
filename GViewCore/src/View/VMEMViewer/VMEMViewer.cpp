#include "VMEMViewer.hpp"

// #include <LexicalViewer/LexicalViewer.hpp>
// #include <include/Internal.hpp>
#include "Internal.hpp"

using namespace GView::View::LexicalViewer;

namespace GView::View::VMEMViewer
{
    Settings::Settings() : data(nullptr) {}
    bool Settings::SetName(std::string_view name) { return true; }

    Instance::Instance(Reference<GView::Object> _obj, Settings* settings)
        : ViewControl("VMEM View")
    {
        obj = _obj;
    }

    void Instance::Paint(AppCUI::Graphics::Renderer& renderer)
    {
        renderer.Clear(' ', ColorPair{Color::Black, Color::Black});
        renderer.WriteSingleLineText(0, 0, "VMEM Viewer", ColorPair{Color::Yellow, Color::Black});
    }

    bool Instance::OnUpdateCommandBar(AppCUI::Application::CommandBar& commandBar)
    {
        commandBar.SetCommand(AppCUI::Input::Key::F2, "Plugins", 0xBF06); // show plugins de la LexicalViewer.hpp
        commandBar.SetCommand(AppCUI::Input::Key::F4, "VMEM View", GView::App::InstanceCommands::CMD_NEXT_VIEW);
        
        return true;
    }

    bool Instance::OnEvent(Reference<Control> ctrl, Event eventType, int ID)
    {
        if (eventType == Event::Command && ID == 0xBF06)
        {
            // dialog smecher de pluginuri
            AppCUI::Dialogs::MessageBox::ShowNotification("Plugins", "Plugin dialog goes here");
            return true;
        }
        return false;
    }
}
