#pragma once
#include "IRenderDevice.h"

namespace GVM
{
class GPUInputLayout
{
public:

    GPUInputLayout(const InputAssemblerDeclarationDesc& desc, int id);
    
    InputAssemblerDeclarationDesc description;
    IRenderDevice::INPUTLAYOUTHANDLE inputLayout = nullptr;
    int id = -1;


};

}
