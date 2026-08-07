#include "VMEMViewer.hpp"

// #include <LexicalViewer/LexicalViewer.hpp>
// #include <include/Internal.hpp>
#include "Internal.hpp"

using namespace GView::View::LexicalViewer;

namespace GView::View::VMEMViewer
{
    class PluginDialog : public Window
    {
      public:
        PluginDialog() : Window("nume plugin dialog", "d:c,w:50,h:14", WindowFlags::ProcessReturn)
        {
            Factory::Label::Create(this, "Asta e un label", "x:1,y:1,w:46,h:1");
            
            Factory::Button::Create(*this, ConstString("&Cancel"), string_view("x:27,y:11,w:12,h:1"), static_cast<int>(Dialogs::Result::Cancel), Controls::ButtonFlags::None);
        }

        bool OnEvent(Reference<Control>, Event eventType, int ID) override
        {
            if (eventType == Event::ButtonClicked)
            {
                this->Exit(Dialogs::Result::Ok); 
                return true;
            }
            if (eventType == Event::WindowClose)
            {
                this->Exit(Dialogs::Result::Cancel);
                return true;
            }
            return false;
        }
    };
    
    Settings::Settings() : data(nullptr) {}
    bool Settings::SetName(std::string_view name) { return true; }

    Instance::Instance(Reference<GView::Object> _obj, Settings* settings)
        : ViewControl("VMEM View")
    {
        obj = _obj;
        sharedState = nullptr;
        if ((settings) && (settings->data))
        {
            sharedState = static_cast<std::string*>(settings->data);
        }
    }

    void Instance::Paint(AppCUI::Graphics::Renderer& renderer)
    {
        renderer.Clear(' ', ColorPair{Color::Black, Color::Black});
        renderer.WriteSingleLineText(0, 0, "VMEM Viewer aici scriu eu", ColorPair{Color::Yellow, Color::Black});
        
        // todo obj type trb file 

        auto to_string = [&](std::u16string_view z){
            std::string r = "";
            for(auto i : z) r += i;
            return r;
        };
        std::string a;
        a = to_string(obj->GetName());
        renderer.WriteSingleLineText(0, 1, "Name: " + a, ColorPair{Color::Yellow, Color::Black});
        a = to_string(obj->GetPath());
        renderer.WriteSingleLineText(0, 2, "Path: " + a, ColorPair{Color::Yellow, Color::Black});

        std::string state = "(no state)";
        if (sharedState)
        {
            state = *sharedState;
        }
        renderer.WriteSingleLineText(0, 3, "State: " + state, ColorPair{ Color::Yellow, Color::Black });
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
            obj->GetContentType()->RunCommand("test");

            // dialog smecher de pluginuri
            // AppCui::Dialogs::
            // AppCUI::Dialogs::MessageBox::ShowNotification("Plugins", "Plugin dialog");
            // return true;

            PluginDialog dlg;
            dlg.Show();
            return true;
        }

        return false;
    }
}
