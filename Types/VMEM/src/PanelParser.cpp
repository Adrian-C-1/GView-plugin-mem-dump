#include "VMEM.hpp"

using namespace GView::Type::VMEM;
using namespace AppCUI::Controls;

constexpr int CMD_BUTTON_REFRESH = 1;
constexpr int BUTTON_HEIGHT      = 2;

// &Parser -> I e rosu
Panels::Parser::Parser(Reference<VMEMFile> _vmem) 
    : TabPage("Dump File Parser") 
{
    vmem = _vmem;

    
    general = Factory::ListView::Create(
        this, "x:0,y:0,w:100%,h:50%", 
        { "n:Field,w:20", "n:Value,w:100" }, 
        ListViewFlags::None
    );

    refreshButton = Factory::Button::Create(this, "&Refresh", "x:0,y:50%,w:20", CMD_BUTTON_REFRESH);
    toStructure = Factory::Button::Create(this, "&Go to Structure", "x:20,y:50%,w:20", CMD_BUTTON_REFRESH + 1);
    
    dataValue = Factory::Label::Create(this, "", "x:0,y:60%,w:100%,h:1");

    details       = Factory::Label::Create(this, "", "x:0,y:90%,w:100%,h:1");
    debug = Factory::Label::Create(this, "Debug aici :3", "x:0,y:99%,w:100%,h:1");

    this->Update();
    this->onInfoUpdate();
}

void Panels::Parser::onInfoUpdate()
{
    general->DeleteAllItems();

    auto structure = this->vmem->dumpAnalyzer->getStructure();
    GView::Utils::ZonesList zones;
    for(int i = 0; i < structure.size(); i++){
        auto &area = structure[i];
        general->AddItem({ area.name, area.value }).SetData(i);
        zones.Add(area.startOffset, area.endOffset - 1, ColorPair{ Color::Black,  i % 2 == 0 ? Color::Aqua : Color::Teal }, area.name);
    }
    vmem->bufferView->SetObjectsHighlightingZonesList(zones);
    vmem->bufferView->OnEvent(nullptr, Event::Command, GView::View::VIEW_COMMAND_ACTIVATE_OBJECT_HIGHLIGHTING);
}

void Panels::Parser::UpdateGeneralInformation()
{
    LocalString<256> tempStr;
    NumericFormatter n;

    int selected_item = general->GetCurrentItem().GetData(0);
    details->SetText(tempStr.Format("Refresh %d | Selected: %d", this->cnt++, selected_item));

    auto structure = this->vmem->dumpAnalyzer->getStructure();
    if (selected_item >= 0 && selected_item < structure.size())
    {
        // valid
        if (debug.IsValid())
            debug->SetText(tempStr.Format("Description: %s", structure[selected_item].description.c_str()));
        
        auto type = structure[selected_item].value;
        if (type == "bool")
        {
            ;
        }
    }
}

void Panels::Parser::RecomputePanelsPositions()
{
    int w = this->GetWidth();
    int h = this->GetHeight();

    if (!general.IsValid())
        return;

    const int listHeight = std::max<>(1, h / 2);
    general->Resize(w, listHeight);

    if (refreshButton.IsValid())
    {
        refreshButton->MoveTo(0, listHeight);
        refreshButton->Resize(20, BUTTON_HEIGHT);
    }

    if (details.IsValid())
    {
        details->MoveTo(0, listHeight + BUTTON_HEIGHT);
        details->Resize(w, std::max<>(1, h - (listHeight + BUTTON_HEIGHT)));
    }
}

bool Panels::Parser::OnEvent(Reference<Control> sender, Event evnt, int controlID)
{
    if (TabPage::OnEvent(sender, evnt, controlID)){
        return true;
    }

    if ((evnt == Event::ButtonClicked) && (controlID == CMD_BUTTON_REFRESH))
    {
        this->Update();
        this->onInfoUpdate();
        return true;
    }
    if ((evnt == Event::ListViewCurrentItemChanged) && (sender == general.ToBase<Control>()))
    {
        auto item = general->GetCurrentItem();
        LocalString<128> tmp;
        int idx = item.GetData(0);
        if (idx == previousListIdex){
            return true;
        }
        
        std::vector<DumpAnalyzer::Area> structure = this->vmem->dumpAnalyzer->getStructure();
        uint64_t off = structure[idx].startOffset;
        if (off >= 0){
            // todo aici fa asta doar daca eventul e triggered by actual list change, nu cursor move din paint (nu vreua sa ma duca la inceput daca doar ma duc in structura cu sagetile)
            if (this->ignoreAlign == true) {
                this->ignoreAlign = false;
            } else {
                GoToAligned(off);
            }
        }
        previousListIdex = idx;

        if (debug.IsValid())
            debug->SetText(tmp.Format("Description: %s", structure[idx].description.c_str()));

        return true;
    }

    if ((evnt == Event::ButtonClicked) && (controlID == CMD_BUTTON_REFRESH + 1))
    {
        // auto tab = this->GetParent().ToObjectRef<Tab>();  
        // tab->SetCurrentTabPageByIndex(1, true);
        
        uint64_t index = this->general->GetCurrentItem().GetData(0);
        bool ok = this->vmem->dumpAnalyzer->goToIndex(index);
        if (ok == false){
            this->debug->SetText(LocalString<128>().Format("Can not go to that structure."));
        }else{
            this->UpdateGeneralInformation();
            this->onInfoUpdate();
        }
        this->details->SetText(LocalString<128>().Format("[buton] Refresh %d | Selected: %d", this->cnt++, index));
        return true;
    }

    return false;
}

void Panels::Parser::Update()
{
    UpdateGeneralInformation();
    RecomputePanelsPositions();
}

void Panels::Parser::Paint(AppCUI::Graphics::Renderer& renderer)
{
    // cand mut cursor in bufferview si am ajuns intr o zona noua
    if (vmem->bufferView.IsValid())
    {
        GView::View::ViewData vd;
        if (vmem->bufferView->GetViewData(vd, GView::Utils::INVALID_OFFSET) &&
            ((vd.cursorStartOffset != lastCursorOffset)))
        {
            lastCursorOffset = vd.cursorStartOffset;
            
            std::vector<DumpAnalyzer::Area> structure = this->vmem->dumpAnalyzer->getStructure();
            for(int i = 0; i < structure.size(); i++){
                if (lastCursorOffset >= structure[i].startOffset && lastCursorOffset < structure[i].endOffset){
                    general->SetCurrentItem(general->GetItem(i));
                    break;
                }
            }

            UpdateGeneralInformation();

            this->ignoreAlign = true;
        }
    }
    TabPage::Paint(renderer);
}

void Panels::Parser::GoToAligned(uint64 offset)
{
    this->lastCursorOffset = offset;

    CHECKRET(vmem->bufferView.IsValid(), "");

    GView::View::ViewData vd;
    CHECKRET(vmem->bufferView->GetViewData(vd, GView::Utils::INVALID_OFFSET), "");

    uint64 BYTES_PER_ROW = 16;

    // aligned la 16 bytes
    const uint64 aligned = offset & ~(BYTES_PER_ROW - 1);
    const uint64 rows  = std::max<uint64>(1, vd.viewSize / BYTES_PER_ROW);
    const uint64 offset_sus   = (rows / 2) * BYTES_PER_ROW;
    const uint64 start = (aligned > offset_sus) ? (aligned - offset_sus) : 0;

    vmem->bufferView->AdvanceStartView((int64) start - (int64) vd.viewStartOffset);
    vmem->bufferView->GoTo(offset);
}