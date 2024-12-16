#pragma once

#include "RenderBase.h"

class RenderPrimitive: public RenderBase
{
public:
    RenderPrimitive() {}
    ~RenderPrimitive()
    {
        Cleanup();
    }

protected:
    uint32_t stride = 0;
    uint32_t offset = 0;
    uint32_t numIndices = 0;
};
