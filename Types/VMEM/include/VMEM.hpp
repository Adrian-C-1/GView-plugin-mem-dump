#pragma once

#include "GView.hpp"

namespace GView
{
namespace Type
{
    namespace VMEM
    {
        class VMEMFile : public TypeInterface{
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
        };
    }
}
}