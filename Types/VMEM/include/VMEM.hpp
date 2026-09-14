#pragma once

#include "GView.hpp"
#include <time.h>
#include <stdarg.h>

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
                Reference<AppCUI::Controls::Button> toBack;
                Reference<AppCUI::Controls::Button> toStructure;
                
                Reference<AppCUI::Controls::Label> description;
                Reference<AppCUI::Controls::Label> dataValue;
                
                Reference<AppCUI::Controls::Label> cursorInfo;
                Reference<AppCUI::Controls::Button> goToPointer;

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
                bool pointerToStructure = false;
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
            private:
                bool virtualMemoryAnalyzer();
            };

            class Debug : public AppCUI::Controls::TabPage
            {
                Reference<VMEMFile> vmem;
                Reference<AppCUI::Controls::Button> refresh;
                Reference<AppCUI::Controls::Label> debug;
            public:
                Debug(Reference<VMEMFile> _vmem);
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
                // Areas for parsing

                struct Area{
                    std::string json = "";
                    std::string name = "";
                    std::string value = "";
                    std::string description = "";
                    std::string jsonPointer = "";
                    uint64_t offset = 0;
                    uint64_t totalSize = 0;
                    std::vector<Area*> children = {};
                    Area* parent = nullptr;
                    bool loaded = false;
                };
                bool goToIndex(uint64_t index);
                bool indexHasChildren(uint64_t index);
                bool goToJsonKey(std::string jsonKey);
                bool goUp();
                bool goToIndex(Area* &area, uint64_t index);
                bool goToJsonKey(Area* &area, std::string jsonKey);
                bool goUp(Area* &area);

                uint64_t getuint64_t(Area* area);
                // 
                
                DumpAnalyzer(Reference<VMEMFile> vmem);
                
                Area* currentArea = nullptr;
                Area* getArea();

                std::vector<char> latestDebug = std::vector<char>(9999999);
                inline void addDebug(const char* format, ...){
                    va_list args;
                    va_start(args, format);
                    vsprintf(latestDebug.data() + strlen(latestDebug.data()), format, args);
                    va_end(args);
                }

                uint8_t* readFromVirtualAddress(uint64_t vaddr, size_t size);
                uint64_t virtualAddressToFileOffset(uint64_t vaddr);    
            private:
                Area* buildArea(std::string jsonKey, Area* parent = nullptr, uint64_t absoluteOffset = 0, uint64_t depth = 1);
                
                /// @brief Parses the runs from the dump file to correctly map memory pages to the dump file offset.
                void loadPagesFromDumpFile();
                uint64_t convertPhysicalAddressToDumpOffset(uint64_t physicalAddress);
                uint64_t virtualToPhysical(uint64_t vaddr);
                uint8_t* readVirtual(uint64_t vaddr, size_t size);
                /// @brief Internal; init buffer inainte sa folosesti 
                void readPhysical(uint64_t physicalAddress, void* buffer, size_t size);
                
                uint64_t dtb;
                uint64_t headerSize;
                struct PhysicalMemoryRun{
                    uint64_t BasePage;
                    uint64_t PageCount;
                };
                std::vector<PhysicalMemoryRun> PhysicalMemoryRuns;

        };
    }
};
}