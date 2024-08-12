#pragma once

#include <Unknwnbase.h>

constexpr void SafeRelease(IUnknown* toRelease)
{
    if (toRelease != nullptr)
        toRelease->Release();
}