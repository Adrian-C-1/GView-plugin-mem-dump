#include "GView.hpp"
#include "VMEM.hpp"
#include "json.hpp"

#include <fstream>

using namespace GView::Type::VMEM;
using namespace nlohmann;

uint64_t calculateJsonEndOffset(uint64_t start, json &haystack, json& needle){
    uint64_t endOffset = start;
    for(auto &area : needle["areas"]){
        auto newJson = haystack[area.get<std::string>()];
        if (newJson["endOffset"] == -1){
            endOffset += calculateJsonEndOffset(0, haystack, newJson);
        }else{
            endOffset += (newJson["endOffset"].get<uint64_t>() - newJson["startOffset"].get<uint64_t>());
        }
    }
    return endOffset;
}
void loadJson(DumpAnalyzer::Area &area){
    std::ifstream f("examples/header64.json");
    json j = json::parse(f);
    std::string json = area.json;

    if (j.contains(json)){
        auto &jsonArea = j[json];
        area.name = jsonArea["name"];
        area.value = jsonArea["value"];
        area.description = jsonArea["description"];
        area.startOffset = jsonArea["startOffset"];
        area.endOffset = jsonArea["endOffset"];
        if (area.endOffset == -1){
            area.endOffset = calculateJsonEndOffset(area.startOffset, j, jsonArea);
        }
        area.loaded = true;
        for(auto &subArea : jsonArea["areas"]){
            area.subAreas.push_back({subArea.get<std::string>()});
        }
        return;
    }

    if (json == "_DUMP_HEADER64"){
        area.name = "Header";
        area.value = "struct _DUMP_HEADER64";
        area.description = "Header of the dump file, contains information about the dump and useful links.";
        area.startOffset = 0x0;
        area.endOffset = 0x2000;
        area.loaded = true;

        area.subAreas.push_back({
            "",
            "Signature",
            "chars",
            "Signature of the dump file, should be 'PAGE'",
            0x0,
            0x4,
            {},
            true
        });
        area.subAreas.push_back({
            "",
            "ValidDump",
            "bool",
            "Indicates if the dump file is valid or not",
            0x4,
            0x5,
            {},
            true
        });
        area.subAreas.push_back({
            "",
            "MajorVersion",
            "uint16_t",
            "Major version of the dump file format",
            0x5,
            0x7,
            {},
            true
        });
        area.subAreas.push_back({
            "",
            "MinorVersion",
            "uint16_t",
            "Minor version of the dump file format",
            0x7,
            0x9,
            {},
            true
        });
        area.subAreas.push_back({
            "",
            "DirectoryTableBase",
            "uint64_t",
            "The base address of the directory table",
            0x9,
            0x11,
            {},
            true
        });
    }
    area.loaded = true;
}

DumpAnalyzer::DumpAnalyzer(Reference<VMEMFile> vmem){
    this->vmem = vmem;
    // aici vmem->obj imi da segfault

    this->loadPagesFromDumpFile();
}
void DumpAnalyzer::buildRootStructure(){
    currentStructure.push_back({
        "_DUMP_HEADER64",
    });
    loadJson(currentStructure[0]);
    currentStructure.push_back({
        "",
        "Dump Data", 
        "sub-structure",
        "contine memoria efectiva a ramului",
        0x2000,
        vmem->obj->GetData().GetSize(),
        {},
        true
    });
}
std::vector<DumpAnalyzer::Area> DumpAnalyzer::getStructure(){
    if (currentStructure.empty()){
        buildRootStructure();
    }

    std::vector<DumpAnalyzer::Area> structure = currentStructure;

    return structure;
}
bool DumpAnalyzer::goToIndex(uint64_t index){
    auto structure = this->getStructure();
    if (index >= structure.size() || index < 0){
        return false;
    }

    auto newStructure = structure[index].subAreas;
    if (newStructure.empty()){
        return false;
    }
    
    this->currentStructure = newStructure;
    for(auto &i : currentStructure){
        if (i.json != "" && i.loaded == false){
            loadJson(i);
        }
    }

    return true;
}

void DumpAnalyzer::loadPagesFromDumpFile(){
    ;
}