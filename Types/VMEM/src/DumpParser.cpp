#include "GView.hpp"
#include "VMEM.hpp"
#include "json.hpp"

#include <fstream>

using namespace GView::Type::VMEM;
using namespace nlohmann;

json& loadJson(){
    std::ifstream f("examples/header64.json");
    static json j = json::parse(f);
    return j;
}

DumpAnalyzer::DumpAnalyzer(Reference<VMEMFile> vmem){
    this->vmem = vmem;
    // aici vmem->obj imi da segfault

    this->headerSize = 0x2000;
    this->currentArea = buildArea("_DUMP64", nullptr, 0, 2);
}

DumpAnalyzer::Area* DumpAnalyzer::getArea(){
    this->loadPagesFromDumpFile();
    return currentArea;
}

/// **************************************************************
/// ***************** Virtual Memory Translation ***************** 
/// **************************************************************

void DumpAnalyzer::loadPagesFromDumpFile(){
    Area* area = buildArea("_DUMP_HEADER64", nullptr, 0, 2);
    const AppCUI::uint8* data;
    
    if (!goToJsonKey(area, "_DUMP_HEADER64_DirectoryTableBase")){
        addDebug("[ERROR] goToJsonKey(area, _DUMP_HEADER64_DirectoryTableBase)\n");
    };
    data = vmem->obj->GetData().Get(area->offset, area->totalSize, 0).GetData();
    memcpy(&this->dtb, data, 8);
    goUp(area);
    
    if (!goToJsonKey(area, "_DUMP_HEADER64_PhysicalMemoryBlockBuffer")){
        addDebug("[ERROR] goToJsonKey(area, _DUMP_HEADER64_PhysicalMemoryBlockBuffer)\n");
    };
    if (!goToJsonKey(area, "_PHYSICAL_MEMORY_DESCRIPTOR64")){
        addDebug("[ERROR] goToJsonKey(area, _PHYSICAL_MEMORY_DESCRIPTOR64)\n");
    };
    

    if (!goToJsonKey(area, "_PHYSICAL_MEMORY_DESCRIPTOR64_NumberOfRuns")){
        addDebug("[ERROR] goToJsonKey(area, _PHYSICAL_MEMORY_DESCRIPTOR64_NumberOfRuns)\n");
    };
    data = vmem->obj->GetData().Get(area->offset, area->totalSize, 0).GetData();
    uint32_t nrRuns = 0; 
    memcpy(&nrRuns, data, 4);
    goUp(area);

    goToJsonKey(area, "_PHYSICAL_MEMORY_DESCRIPTOR64_NumberOfPages");
    data = vmem->obj->GetData().Get(area->offset, area->totalSize, 0).GetData();
    uint64_t nrPages = 0;
    memcpy(&nrPages, data, 8);
    goUp(area);

    goToJsonKey(area, "_PHYSICAL_MEMORY_DESCRIPTOR64_Run");
    PhysicalMemoryRuns.clear();
    for(int i = 0; i < nrRuns; i++){
        goToIndex(area, i); // _PHYSICAL_MEMORY_RUN64
        
        goToJsonKey(area, "_PHYSICAL_MEMORY_RUN64_BasePage");
        data = vmem->obj->GetData().Get(area->offset, area->totalSize, 0).GetData();
        uint64_t basePage = 0;
        memcpy(&basePage, data, 8);
        goUp(area);

        goToJsonKey(area, "_PHYSICAL_MEMORY_RUN64_PageCount");
        data = vmem->obj->GetData().Get(area->offset, area->totalSize, 0).GetData();
        uint64_t pageCount = 0;
        memcpy(&pageCount, data, 8);
        goUp(area);

        goUp(area); // index

        this->PhysicalMemoryRuns.push_back({basePage, pageCount});
    }

    latestDebug.data()[0] = 0;
    sprintf(latestDebug.data() + strlen(latestDebug.data()), "nrRuns: %d, nrPages: %lld\n", nrRuns, nrPages);
    int cnt = 0;
    for(auto &i : PhysicalMemoryRuns){
        sprintf(latestDebug.data() + strlen(latestDebug.data()), "index: %d:  base: %lld, count: %lld\n", cnt++, i.BasePage, i.PageCount);
    }
}
uint64_t DumpAnalyzer::convertPhysicalAddressToDumpOffset(uint64_t physicalAddress){
    uint64_t page = physicalAddress >> 12;
    uint64_t offset = physicalAddress & 0xFFF; // 4KiB granular page = 2^12
    uint64_t realPageIndex = 0; // !

    for(auto &run : this->PhysicalMemoryRuns){ 
        if (page >= run.BasePage && page < run.BasePage + run.PageCount){
            uint64_t pageIndexInRun = page - run.BasePage;
            uint64_t realPageIndexFinal = realPageIndex + pageIndexInRun;
            uint64_t fileOffset = headerSize + realPageIndexFinal * 0x1000 + offset;
            return fileOffset;
        }
        realPageIndex += run.PageCount; // dumpul mapreaza memoria tight dar mia are gapuri
    }
    return 0;
}
uint64_t DumpAnalyzer::virtualToPhysical(uint64_t vaddr){
    std::ifstream f("assets/mem.dmp");
    // https://wiki.osdev.org/X86_Paging#Page_Table
    // https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.pdf
    // pag 111
    // todo momentan presupun IA-32e x86-64, 4 level paging

    uint64_t address_mask = 0x000FFFFFFFFFF000ULL;
    uint64_t present_mask = 0x1;
    auto page_size_bit_set = [&](uint64_t entry) -> bool{
        return entry & (1ull << 7);
    }; 

    // pml4
    uint64_t l4_table = dtb & address_mask;

    uint64_t l4_index = (vaddr >> 39) & 0b111111111;
    uint64_t l4_where = l4_table + l4_index * 8;
    
    uint64_t l4_entry = 0;
    readPhysical(l4_where, &l4_entry, 8);
    if (!l4_entry & present_mask){
        return 0;
    }
    if (page_size_bit_set(l4_entry)){
        return 0;
    }

    // pdpt
    uint64_t l3_table = l4_entry & address_mask;

    uint64_t l3_index = (vaddr >> 30) & 0b111111111;
    uint64_t l3_where = l3_table + l3_index * 8;

    uint64_t l3_entry = 0;
    readPhysical(l3_where, &l3_entry, 8);
    if (!l3_entry & present_mask){
        return 0;
    }
    if (page_size_bit_set(l3_entry)){
        return (l3_entry & address_mask) | (vaddr & 0x3FFFFFFF);
    }

    // pd
    uint64_t l2_table = l3_entry & address_mask;
    
    uint64_t l2_index = (vaddr >> 21) & 0b111111111;
    uint64_t l2_where = l2_table + l2_index * 8;
    
    uint64_t l2_entry = 0;
    readPhysical(l2_where, &l2_entry, 8);
    if (!l2_entry & present_mask){
        return 0;
    }
    if (page_size_bit_set(l2_entry)){
        return (l2_entry & address_mask) | (vaddr & 0x1FFFFF);
    }
    
    // pt
    uint64_t l1_table = l2_entry & address_mask;

    uint64_t l1_index = (vaddr >> 12) & 0b111111111;
    uint64_t l1_where = l1_table + l1_index * 8;
    
    uint64_t l1_entry = 0;
    readPhysical(l1_where, &l1_entry, 8);
    if (!l1_entry & present_mask){
        return 0;
    }
    // PS nu conteaza aici, e 4 KiB in ambele cazuri
    return (l1_entry & address_mask) | (vaddr & 0xFFF);
}
uint8_t* DumpAnalyzer::readVirtual(uint64_t vaddr, size_t size){
    uint8_t* ret = new uint8_t[size];
    uint64_t read = 0;
    while (read < size){
        uint64_t untilPageEnd = 0x1000 - (vaddr & 0xFFF);
        uint64_t toRead = std::min(untilPageEnd, size - read);

        uint64_t phys = virtualToPhysical(vaddr);
        if (phys == 0){
            return nullptr;
        }
        uint64_t fileOffset = convertPhysicalAddressToDumpOffset(phys);
        AppCUI::Utils::BufferView b = vmem->obj->GetData().Get(fileOffset, toRead, 0);
        memcpy(ret + read, b.GetData(), toRead);
        
        read += toRead;
        vaddr += toRead;
    }
    return ret;
}
void DumpAnalyzer::readPhysical(uint64_t physicalAddress, void* buffer, size_t size) {
    uint8_t* dest = reinterpret_cast<uint8_t*>(buffer);

    uint64_t read = 0;
    while (read < size)
    {
        uint64_t untilPageEnd = 0x1000 - (physicalAddress & 0xFFF);
        uint64_t toRead = std::min(untilPageEnd, size - read);

        uint64_t fileOffset = convertPhysicalAddressToDumpOffset(physicalAddress);
        if (fileOffset == 0)
            return;

        AppCUI::Utils::BufferView b =
            vmem->obj->GetData().Get(fileOffset, toRead, 0);

        memcpy(dest + read, b.GetData(), toRead);

        read += toRead;
        physicalAddress += toRead;
    }
}
uint8_t* DumpAnalyzer::readFromVirtualAddress(uint64_t vaddr, size_t size){
    return readVirtual(vaddr, size);
}
uint64_t DumpAnalyzer::virtualAddressToFileOffset(uint64_t vaddr){
    uint64_t phys = virtualToPhysical(vaddr);
    if (phys == 0){
        return 0;
    }
    return convertPhysicalAddressToDumpOffset(phys);
}

/// **************************************************************
/// **************************** Area **************************** 
/// **************************************************************

DumpAnalyzer::Area* DumpAnalyzer::buildArea(std::string jsonKey, Area* parent, uint64_t absoluteOffset, uint64_t depth){
    if (depth == 0){
        return nullptr;
    }

    auto area = new Area;
    area->json = jsonKey;
    area->parent = parent;
    area->offset = absoluteOffset;
    
    json& j = loadJson();
    if (!j.contains(jsonKey)) {
        return area;
    }
    
    auto& jsonArea = j[jsonKey];
    area->name = jsonArea["name"];
    area->value = jsonArea["value"];
    area->description = jsonArea["description"];
    area->totalSize = jsonArea["totalSize"];
    area->jsonPointer = jsonArea.value("jsonPointer", "");
    area->loaded = true;
    
    size_t pos = area->value.find("[");
    size_t pos_end = area->value.find("]");
    if (pos != std::string::npos) {
        // este un array actually
        std::string type = area->value.substr(0, pos);
        std::string count = area->value.substr(pos + 1, pos_end - pos - 1);
        if (j.contains(type)){
            // este char un array de structuri
            uint64_t count_n = std::stoull(count);
            if (count_n == 0) count_n = 1;
            uint64_t size_per_item = area->totalSize / count_n;
            for(uint64_t i = 0; i < count_n; i++){
                auto child = buildArea(type, area, area->offset + size_per_item * i);
                area->children.push_back(child);
            }
        }
    }else{
        // nu este un array, ci whatever e in json thats it
        for(auto &reference : jsonArea["areas"]){
            auto child = buildArea(reference["json"].get<std::string>(), area, area->offset + reference["offset"].get<uint64_t>(), depth - 1);
            area->children.push_back(child);
        }
    }

    return area;
}
// todo suport pt jsonPointer
bool DumpAnalyzer::goToIndex(Area* &area, uint64_t index){
    if (index < 0 || index >= area->children.size()){
        return false;
    }
    Area* child = area->children[index];

    if (child->jsonPointer.size() > 0){
        // E posibil ca childul sa fie un pointer catre o structura, caz in care mergem la ea.
        // Ex PsActiveModuleList
        // todo in loc de 0 sa pun offsetul corect din vmem to offset
        // todo goto pointer si go to parent pune numele prost
        Area* newArea = buildArea(child->jsonPointer, area, 0, 2);
        area->children[index] = newArea;
        area = area->children[index];
        return true;
    }

    Area* newArea = buildArea(child->json, area, child->offset, 2);
    area->children[index] = newArea;
    area = area->children[index];
    return true;
}
bool DumpAnalyzer::goToJsonKey(Area* &area, std::string jsonKey){
    for(int i = 0; i < area->children.size(); i++){
        if (area->children[i]->json == jsonKey){
            return this->goToIndex(area, i);
        }
    }
    return false;
}
bool DumpAnalyzer::goUp(Area* &area){
    if (area->parent == nullptr){
        return false;
    }
    area = area->parent;
    // todo unload child? 
    return true;
}

bool DumpAnalyzer::goToIndex(uint64_t index){
    return goToIndex(currentArea, index);
}
bool DumpAnalyzer::indexHasChildren(uint64_t index){
    if (index < 0 || index >= currentArea->children.size()){
        return false;
    }
    return !currentArea->children[index]->children.empty();
}
bool DumpAnalyzer::goToJsonKey(std::string jsonKey){
    return goToJsonKey(currentArea, jsonKey);
}
bool DumpAnalyzer::goUp(){
    return this->goUp(currentArea);
}

uint64_t DumpAnalyzer::getuint64_t(Area* area){
    return 0x0;
}