#pragma once
#include "Internal.hpp"

namespace GView::View::VMEMViewer
{

    class Instance : public View::ViewControl
    {
        Reference<GView::Object> obj;
        String name;
    public:
        Instance(Reference<GView::Object> obj, Settings* settings);

        // virtuale ViewControl
        bool GoTo(uint64 offset) override { return false; }
        bool Select(uint64 offset, uint64 size) override { return false; }
        bool ShowGoToDialog() override { return false; }
        bool ShowFindDialog() override { return false; }
        bool ShowCopyDialog() override { return false; }
        void PaintCursorInformation(AppCUI::Graphics::Renderer&, uint32, uint32) override {}

        // asta !
        void Paint(AppCUI::Graphics::Renderer& renderer) override;
        bool OnUpdateCommandBar(AppCUI::Application::CommandBar& commandBar) override;
        bool OnEvent(Reference<Control>, Event eventType, int ID) override;

        // PropertiesInterface ?
        bool GetPropertyValue(uint32 id, PropertyValue& value) override { return false; }
        bool SetPropertyValue(uint32 id, const PropertyValue& value, String& error) override { return false; }
        void SetCustomPropertyValue(uint32 propertyID) override {}
        bool IsPropertyValueReadOnly(uint32 propertyID) override { return true; }
        const vector<Property> GetPropertiesList() override { return {}; }

        std::string_view GetCategoryNameForSerialization() const override { return "View.VMEM"; }
        bool AddCategoryBeforePropertyNameWhenSerializing() const override { return true; }

    };
}
