#pragma once

#include "GView.hpp"
#include <time.h>

namespace GView
{
namespace Type
{
    namespace VMEM
    {
        class DumpAnalyzer;

        class VMEMFile : public TypeInterface {
            public:
                VMEMFile();
                std::string_view GetTypeName() override;
                void RunCommand(std::string_view commandName) override;
                bool UpdateKeys(KeyboardControlsInterface* interface) override;
                Utils::JsonBuilderInterface* GetSmartAssistantContext(const std::string_view& prompt, std::string_view displayPrompt) override;

                // Furate de la XML

                Reference<GView::Utils::SelectionZoneInterface> selectionZoneInterface;

                uint32 GetSelectionZonesCount() override
                {
                    CHECK(selectionZoneInterface.IsValid(), 0, "");
                    return selectionZoneInterface->GetSelectionZonesCount();
                }

                TypeInterface::SelectionZone GetSelectionZone(uint32 index) override
                {
                    static auto d = TypeInterface::SelectionZone{ 0, 0 };
                    CHECK(selectionZoneInterface.IsValid(), d, "");
                    CHECK(index < selectionZoneInterface->GetSelectionZonesCount(), d, "");

                    return selectionZoneInterface->GetSelectionZone(index);
                }
            public:
                std::string a = "a";
            
            public:
                Reference<GView::View::WindowInterface> win = nullptr;
                Reference<GView::View::ViewControl> bufferView = nullptr;
                DumpAnalyzer* dumpAnalyzer = nullptr;
        };

        namespace Panels
        {
            class Information : public AppCUI::Controls::TabPage
            {
                Reference<VMEMFile> vmem;
                Reference<AppCUI::Controls::ListView> general;
                Reference<AppCUI::Controls::Button> refreshButton;
                Reference<AppCUI::Controls::Label> details;
                Reference<AppCUI::Controls::Button> toStructure;

                Reference<AppCUI::Controls::Label> debug;

                void UpdateGeneralInformation();
                void RecomputePanelsPositions();

                int cnt = 0;
            public:
                Information(Reference<VMEMFile> vmem);
                void Update();
                virtual void OnAfterResize(int newWidth, int newHeight) override
                {
                    RecomputePanelsPositions();
                }
                virtual bool OnEvent(
                      Reference<AppCUI::Controls::Control> sender, AppCUI::Controls::Event evnt, int controlID) override;
                uint64_t lastCursorOffset = GView::Utils::INVALID_OFFSET;
                virtual void Paint(AppCUI::Graphics::Renderer& renderer) override;
            private:
                void GoToAligned(uint64 offset);
                time_t lastClickTime = 0;
            };
            class Structure : public AppCUI::Controls::TabPage
            {
                Reference<VMEMFile> vmem;
                Reference<AppCUI::Controls::Button> back;
            public:
                void Update();
                Structure(Reference<VMEMFile> vmem);
                virtual void OnAfterResize(int newWidth, int newHeight) override;
                virtual bool OnEvent(
                      Reference<AppCUI::Controls::Control> sender, AppCUI::Controls::Event evnt, int controlID) override;
                virtual void Paint(AppCUI::Graphics::Renderer& renderer) override;
            };
        }

        class DumpAnalyzer{
            private:
                Reference<VMEMFile> vmem;
            public:
                DumpAnalyzer(Reference<VMEMFile> vmem);
                std::vector<std::pair<std::string, std::string>> getHeaderFields();
        };
    }
}
}