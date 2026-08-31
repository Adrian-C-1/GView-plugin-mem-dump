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

    this->currentArea = buildArea("_DUMP64", nullptr, 0, 2);
}

DumpAnalyzer::Area* DumpAnalyzer::getArea(){
    this->loadPagesFromDumpFile();
    return currentArea;
}

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

bool DumpAnalyzer::goToIndex(Area* &area, uint64_t index){
    if (index < 0 || index >= area->children.size()){
        return false;
    }
    Area* child = area->children[index];
    if (child->children.empty()){
        // return false;
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