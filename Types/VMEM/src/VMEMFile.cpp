#include "VMEM.hpp"

using namespace GView::Type::VMEM;

VMEMFile::VMEMFile(){};
std::string_view VMEMFile::GetTypeName() {
    return "VMEM";
};
void VMEMFile::RunCommand(std::string_view commandName) {
};
bool VMEMFile::UpdateKeys(KeyboardControlsInterface* interface) {
    return true;
};
GView::Utils::JsonBuilderInterface* VMEMFile::GetSmartAssistantContext(const std::string_view& prompt, std::string_view displayPrompt) {
    return nullptr;
};