#include "VMEM.hpp"

using namespace GView::Type::VMEM;

VMEMFile::VMEMFile(){
    this->dumpAnalyzer = new DumpAnalyzer(this);
};
std::string_view VMEMFile::GetTypeName() {
    return "VMEM";
};
void VMEMFile::RunCommand(std::string_view commandName) {
    if (commandName == "test")
    {
        this->a = "b";
    }
};
bool VMEMFile::UpdateKeys(KeyboardControlsInterface* interface) {
    return true;
};
GView::Utils::JsonBuilderInterface* VMEMFile::GetSmartAssistantContext(const std::string_view& prompt, std::string_view displayPrompt) {
    return nullptr;
    // auto builder = GView::Utils::JsonBuilderInterface::Create();
    // builder->AddU16String("Name", obj->GetName());
    // builder->AddUInt("ContentSize", obj->GetData().GetSize());
    // return builder;
};