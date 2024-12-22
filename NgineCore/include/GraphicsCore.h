#pragma once
#include "Core.hxx"
#include "Window.h"
#include "Exception.h"

namespace Ngine
{
#if defined(TARGET_PLATFORM_LINUX)
    class GraphicsCore
    {
    public:
        GraphicsCore(Window* pWindow);
        ~GraphicsCore();

    private:
        void CreateInstance();
        void PickPhysicalDevice(uint32_t devIndex);
        void AutoPickPhysicalDevice();
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
        bool CheckValidationSupport();
        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void SetupDebugMessenger();

    private:
        VkInstance mInstance;
        VkPhysicalDevice mPhysDevice;
        VkDevice mDevice;
        VkDebugUtilsMessengerEXT mDebugMessenger;
    };
#elif defined(TARGET_PLATFORM_WINDOWS)

#elif defined(TARGET_PLATFORM_XBOX)
    class NGAPI GraphicsCore
    {
    public:
        GraphicsCore(Window* pWindow);
        ~GraphicsCore();

    private:
    };

#endif
}