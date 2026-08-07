#pragma once

#include "GView.hpp"

namespace GView
{
namespace Type
{
    namespace VMEM
    {
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
        };

        namespace Panels
        {
            class Information : public AppCUI::Controls::TabPage
            {
                Reference<VMEMFile> vmem;
                Reference<AppCUI::Controls::ListView> general;

                void UpdateGeneralInformation();
                void RecomputePanelsPositions();

            public:
                Information(Reference<VMEMFile> vmem);
                void Update();
                virtual void OnAfterResize(int newWidth, int newHeight) override
                {
                    RecomputePanelsPositions();
                }
            };
        }

        namespace Views
        {
            class VMEMView : public GView::View::ViewControl
            {
                Reference<VMEMFile> vmem;
            public:
                VMEMView(Reference<VMEMFile> vmem);
                
                bool GoTo(uint64 offset) override { return false; }
                bool Select(uint64 offset, uint64 size) override { return false; }
                bool ShowGoToDialog() override { return false; }
                bool ShowFindDialog() override { return false; }
                bool ShowCopyDialog() override { return false; }
                void PaintCursorInformation(AppCUI::Graphics::Renderer& renderer, uint32 width, uint32 height) override {}
                
                void Paint(AppCUI::Graphics::Renderer& renderer) override;
            };
        }
    }
}
}