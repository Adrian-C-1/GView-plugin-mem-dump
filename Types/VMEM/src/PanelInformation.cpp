#include "VMEM.hpp"

using namespace GView::Type::VMEM;
using namespace AppCUI::Controls;

constexpr int CMD_BUTTON_REFRESH = 1;
constexpr int BUTTON_HEIGHT      = 2;

// &Information -> I e rosu
Panels::Information::Information(Reference<VMEMFile> _vmem) 
    : TabPage("Dump File Parser") 
{
    vmem = _vmem;

    
    general = Factory::ListView::Create(
        this, "x:0,y:0,w:100%,h:50%", 
        { "n:Field,w:20", "n:Value,w:100" }, 
        ListViewFlags::None
    );

    refreshButton = Factory::Button::Create(this, "&Refresh", "x:0,y:50%,w:20", CMD_BUTTON_REFRESH);
    details       = Factory::Label::Create(this, "", "x:0,y:50%,w:100%,h:1");
    toStructure = Factory::Button::Create(this, "&Go to Structure", "x:20,y:50%,w:20", CMD_BUTTON_REFRESH + 1);

    debug = Factory::Label::Create(this, "Debug aici :3", "x:0,y:99%,w:100%,h:1");

    this->Update();
}

void Panels::Information::UpdateGeneralInformation()
{
    general->DeleteAllItems();

    LocalString<256> tempStr;
    NumericFormatter n;

    general->AddItem("File Info").SetType(ListViewItem::Type::Category);

    auto fields = this->vmem->dumpAnalyzer->getHeaderFields();

    int cnt = 0;
    for(auto &[name, value] : fields){
        general->AddItem({ name, value }).SetData(cnt + 1000);
        cnt += 1;
    }
    general->AddItem({ "Cnt", std::to_string(++this->cnt) });

    if (details.IsValid())
        details->SetText(tempStr.Format("Refresh %d", this->cnt));
}

void Panels::Information::RecomputePanelsPositions()
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

bool Panels::Information::OnEvent(Reference<Control> sender, Event evnt, int controlID)
{
    if (TabPage::OnEvent(sender, evnt, controlID)){
        return true;
    }

    if ((evnt == Event::ButtonClicked) && (controlID == CMD_BUTTON_REFRESH))
    {
        this->Update();
        return true;
    }
    if ((evnt == Event::ListViewCurrentItemChanged) && (sender == general.ToBase<Control>()))
    {
        auto item = general->GetCurrentItem();
        LocalString<128> tmp;
        details->SetText(tmp.Format("Data: %d", item.GetData(0)));
        int off = item.GetData(0) - 1000;
        if (off >= 0){
            GoToAligned(off);
        }

        // todo cum fac asta?
        if (std::time(nullptr) - lastClickTime <= 1){
            this->debug->SetText(LocalString<128>().Format("[%ld] d click on item w data: %d", lastClickTime, item.GetData(0)));
        }else{
            this->debug->SetText(LocalString<128>().Format("[%ld] click on item w data: %d", lastClickTime, item.GetData(0)));
        }
        lastClickTime = std::time(nullptr);
        
        return true;
    }

    if ((evnt == Event::ButtonClicked) && (controlID == CMD_BUTTON_REFRESH + 1))
    {
        // this->vmem->win->AddPanel(AppCUI::Utils::Pointer<TabPage>(new Panels::Structure(vmem)), true);
        auto tab = this->GetParent().ToObjectRef<Tab>();  
        tab->SetCurrentTabPageByIndex(1, true);
        return true;
    }

    return false;
}

void Panels::Information::Update()
{
    UpdateGeneralInformation();
    RecomputePanelsPositions();
}

void Panels::Information::Paint(AppCUI::Graphics::Renderer& renderer)
{
    if (vmem->bufferView.IsValid())
    {
        GView::View::ViewData vd;
        if (vmem->bufferView->GetViewData(vd, GView::Utils::INVALID_OFFSET) &&
            ((vd.cursorStartOffset != lastCursorOffset)))
        {
            lastCursorOffset = vd.cursorStartOffset;
            UpdateGeneralInformation();

            this->details->SetText(
                LocalString<128>().Format("Refresh %d offset: 0x%llX", this->cnt, lastCursorOffset)
            );
            for(int i = 0; i < general->GetItemsCount(); i++)
            {
                auto item = general->GetItem(i);
                int off   = item.GetData(0) - 1000;
                if (off == lastCursorOffset)
                {
                    general->SetCurrentItem(item);
                    break;
                }
            }
        }
    }
    TabPage::Paint(renderer);
}

void Panels::Information::GoToAligned(uint64 offset)
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