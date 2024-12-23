#pragma once
#include "Core.hxx"
#include "Window.h"
#include "Exception.h"

namespace Ngine
{
#if defined(TARGET_PLATFORM_LINUX)
    class GraphicsCore
    {
    private:
        class QueueFamilyData
        {
        public:
            std::optional<uint32_t> mGraphicsQueueIndex;
            std::optional<uint32_t> mPresentationQueueIndex;
        };
    public:
        GraphicsCore(NgineWindow* pWindow);
        ~GraphicsCore();

    private:
        void CreateInstance();
        void PickPhysicalDevice(uint32_t devIndex);
        void AutoPickPhysicalDevice();
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
        bool CheckValidationSupport();
        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void SetupDebugMessenger();
        void CreateSurface(NgineWindow* pWindow);
        void ObtainQueueIndexes();

    private:
        VkInstance mInstance;
        VkPhysicalDevice mPhysDevice;
        VkDevice mDevice;
        VkDebugUtilsMessengerEXT mDebugMessenger;
        VkSurfaceKHR mSurface;
        QueueFamilyData mQueueData;
    };
#elif defined(TARGET_PLATFORM_WINDOWS)

#elif defined(TARGET_PLATFORM_XBOX)
    class NGAPI GraphicsCore
    {
    public:
        GraphicsCore(NgineWindow* pWindow);
        ~GraphicsCore();

    private:
    };

#endif
}