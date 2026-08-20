#include "GView.hpp"
#include "VMEM.hpp"

namespace GView::Type::VMEM{
    DumpAnalyzer::DumpAnalyzer(Reference<VMEMFile> vmem){
        this->vmem = vmem;
    }
    std::vector<std::pair<std::string, std::string>> DumpAnalyzer::getHeaderFields() {
        std::vector<std::pair<std::string, std::string>> headerFields;
        std::u16string_view name = vmem->obj->GetName();
        headerFields.push_back({"ObjName", std::string(name.begin(), name.end())});
        headerFields.push_back({"ObjSize", std::to_string(vmem->obj->GetData().GetSize())});
        std::stringstream ss;
        BufferView data = vmem->obj->GetData().Get(0, 10, false);
        ss << std::hex;
        for (int i = 0; i < 10; i++) {
            ss << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
        }
        headerFields.push_back({"ObjData", ss.str()});
        for(int i = 0; i < 350; i++){
            headerFields.push_back({"TestField" + std::to_string(i), "TestValue" + std::to_string(i)});
        }
        return headerFields;
    }
}