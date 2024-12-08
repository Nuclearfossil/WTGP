#pragma once
#include <cstdint>

class RenderBase
{
public:
    RenderBase();
    ~RenderBase();

    virtual void Cleanup() {};
};