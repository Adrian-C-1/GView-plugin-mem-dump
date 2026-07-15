#include "VMEM.hpp"

using namespace GView::Type::VMEM::Views;

// void VMEMView::Paint(AppCUI::Graphics::Renderer& renderer)
// {
//     renderer.WriteSingleLineText(0, 0, "Hello from my view!", ColorPair{Color::White, Color::Black});
// };

VMEMView::VMEMView(Reference<VMEMFile> _vmem) 
    : ViewControl("VMEM View")
{
    vmem = _vmem;
}

void VMEMView::Paint(AppCUI::Graphics::Renderer& renderer)
{
    renderer.Clear(' ', ColorPair{Color::White, Color::Black});
    renderer.WriteSingleLineText(0, 0, "Hello from VMEM view", ColorPair{Color::Yellow, Color::Black});
}
