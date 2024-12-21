#pragma once
#include "Core.hxx"
#include "Window.h"

namespace Ngine
{
#if defined(__linux__) || defined(linux)
    class GraphicsCore
    {
    public:
        GraphicsCore(Window* pWindow);
        ~GraphicsCore();

    private:
        void CreateInstance();

    private:
        VkInstance mInstance;
    }
#endif
}