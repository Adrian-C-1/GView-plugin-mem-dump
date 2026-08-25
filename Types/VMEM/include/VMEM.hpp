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
            class Parser : public AppCUI::Controls::TabPage
            {
                Reference<VMEMFile> vmem;
                Reference<AppCUI::Controls::ListView> general;
                Reference<AppCUI::Controls::Button> refreshButton;
                Reference<AppCUI::Controls::Button> toStructure;
                
                Reference<AppCUI::Controls::Label> dataValue;

                Reference<AppCUI::Controls::Label> details;
                Reference<AppCUI::Controls::Label> debug;

                void UpdateGeneralInformation();
                void RecomputePanelsPositions();

                int cnt = 0;
            public:
                Parser(Reference<VMEMFile> vmem);
                void Update();
                virtual void OnAfterResize(int newWidth, int newHeight) override
                {
                    RecomputePanelsPositions();
                }
                virtual bool OnEvent(
                      Reference<AppCUI::Controls::Control> sender, AppCUI::Controls::Event evnt, int controlID) override;
                uint64_t lastCursorOffset = GView::Utils::INVALID_OFFSET;
                uint64_t previousListIdex = 0;
                bool ignoreAlign = false;
                virtual void Paint(AppCUI::Graphics::Renderer& renderer) override;
                void onInfoUpdate();
            private:
                void GoToAligned(uint64 offset);
            };

            class Plugins : public AppCUI::Controls::TabPage
            {
                Reference<VMEMFile> vmem;
                Reference<AppCUI::Controls::ListView> general;

                Reference<AppCUI::Controls::Button> inputLabel;
                Reference<AppCUI::Controls::TextField> input;

                Reference<AppCUI::Controls::Button> submit;

                Reference<AppCUI::Controls::Label> debug;
            public:
                void Update();
                Plugins(Reference<VMEMFile> vmem);
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
            // todo copac cu lazy loading? 
                struct Area{
                    std::string json = "";
                    std::string name = "";
                    std::string value = "";
                    std::string description = "";
                    uint64_t startOffset = 0; // inclusive
                    uint64_t endOffset = 0; // exclusive
                    std::vector<Area> subAreas = {};
                    bool loaded = false;
                };
                DumpAnalyzer(Reference<VMEMFile> vmem);
                void buildRootStructure();
                std::vector<Area> getStructure();
                bool goToIndex(uint64_t index);
                
                std::vector<Area> currentStructure = {};

            // Memory translation, pages etc
            private:
                void loadPagesFromDumpFile();
                
        };
    }
}
}