#include "GraphicsCore.h"
#include <vulkan/vulkan_core.h>

namespace Ngine
{
    GraphicsCore::GraphicsCore(Window* pWindow)
    {
        CreateInstance();
    }

    GraphicsCore::~GraphicsCore()
    {
        vkDestroyInstance(mInstance, nullptr);
    }

    void GraphicsCore::CreateInstance()
    {
        uint32_t extCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&extCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extCount);
        LOG_F(INFO, "Number of instance extensions: %d", extensions.size());

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
        appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
        appInfo.apiVersion = VK_API_VERSION_1_3;
        appInfo.pEngineName = "Ngine";
        appInfo.pApplicationName = "Ngine Linux Runtime";

        VkInstanceCreateInfo instInfo = {};
        instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instInfo.pApplicationInfo = &appInfo;
        instInfo.enabledExtensionCount = extensions.size();
        instInfo.ppEnabledExtensionNames = extensions.data();
        instInfo.enabledLayerCount = 0;

        VkResult res = vkCreateInstance(&instInfo, nullptr, &mInstance);

    }
}