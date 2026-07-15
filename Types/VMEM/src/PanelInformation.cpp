#include "VMEM.hpp"

using namespace GView::Type::VMEM;
using namespace AppCUI::Controls;

Panels::Information::Information(Reference<VMEMFile> _vmem) 
    : TabPage("&Information") 
{
    vmem = _vmem;

    general = Factory::ListView::Create(this, "x:0,y:0,w:100%,h:10", { "n:Field,w:20", "n:Value,w:100" }, ListViewFlags::None);


    this->Update();
}

void Panels::Information::UpdateGeneralInformation()
{
    general->DeleteAllItems();

    LocalString<256> tempStr;
    NumericFormatter n;

    general->AddItem("File Info").SetType(ListViewItem::Type::Category);

    general->AddItem({ 
        "Name", 
        vmem->obj->GetName() 
    });

    general->AddItem({ 
        "Size", 
        tempStr.Format(
            "%s bytes", 
            n.ToString(
                vmem->obj->GetData().GetSize(), 
                { NumericFormatFlags::None, 10, 3, ',' }
            ).data()
        ) 
    });

    general->AddItem({ "Type", "VMEM" });

    general->AddItem({ "Alta chestie de aici", "Alta valoare random" });
}

void Panels::Information::RecomputePanelsPositions()
{
    int w = this->GetWidth();
    int h = this->GetHeight();

    if (!general.IsValid())
        return;

    general->Resize(w, h);
}

void Panels::Information::Update()
{
    UpdateGeneralInformation();
    RecomputePanelsPositions();
}
