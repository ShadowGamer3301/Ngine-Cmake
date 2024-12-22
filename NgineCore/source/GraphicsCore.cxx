#include "GraphicsCore.h"
#include "Core.hxx"
#include "Exception.h"
#include "FileUtils.h"
#include <vulkan/vulkan_core.h>

namespace Ngine
{
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }


    GraphicsCore::GraphicsCore(Window* pWindow)
    {
        CreateInstance();

        bool devAutoPick = FileUtils::GetBoolFromConfig("Resource/ngine.ini", "General", "AutoPickDevice");
        int devIndex = FileUtils::GetIntegerFromConfig("Resource/ngine.ini", "General", "ManualDeviceIndex");
        
        if(devAutoPick)
            AutoPickPhysicalDevice();
        else
            PickPhysicalDevice(devIndex);
    }

    GraphicsCore::~GraphicsCore()
    {
        vkDestroyInstance(mInstance, nullptr);
    }

    void GraphicsCore::CreateInstance()
    {
        bool enableVL = FileUtils::GetBoolFromConfig("Resource/ngine.ini", "General", "EnableGfxDebugMode");

        //Check if device supports validation layers
        if(enableVL && !CheckValidationSupport())
        {
            //If device does not support VL turn them off
            LOG_F(ERROR,"Validation layers were requested but are not supported on this device!");
            enableVL = false;
        }

        LOG_IF_F(WARNING, enableVL, "Validation layers enabled!");

        uint32_t extCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&extCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extCount);

        if (enableVL)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        LOG_F(INFO, "Number of instance extensions: %zu", extensions.size());

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
        appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
        appInfo.apiVersion = VK_API_VERSION_1_3;
        appInfo.pEngineName = "Ngine";
        appInfo.pApplicationName = "Ngine Linux Runtime";

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

        VkInstanceCreateInfo instInfo = {};
        instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instInfo.pApplicationInfo = &appInfo;
        instInfo.enabledExtensionCount = extensions.size();
        instInfo.ppEnabledExtensionNames = extensions.data();
        if(enableVL)
        {
            instInfo.enabledLayerCount = validationLayers.size();
            instInfo.ppEnabledLayerNames = validationLayers.data();
            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            instInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            instInfo.enabledLayerCount = 0;
            instInfo.ppEnabledLayerNames = nullptr;
        }

        VkResult res = vkCreateInstance(&instInfo, nullptr, &mInstance);
        VK_THROW_IF_FAILED(res);

    }

    void GraphicsCore::AutoPickPhysicalDevice()
    {
        //Count all avaliable devices
        uint32_t devCount = 0;
        vkEnumeratePhysicalDevices(mInstance, &devCount, nullptr);

        //If there isn't at least one device throw exception
        if(devCount < 1)
        {
            LOG_F(ERROR, "There are no avaliable devices on your system!");
            throw Exception();
        }

        //Obtain all devices
        std::vector<VkPhysicalDevice> vecDevices(devCount);
        vkEnumeratePhysicalDevices(mInstance, &devCount, &vecDevices[0]);

        //Enumerate through all the devices and select the most suitable one
        for(auto& device : vecDevices)
        {
            VkPhysicalDeviceProperties devProp;
            vkGetPhysicalDeviceProperties(device, &devProp);

            //Ignore any device that doesn't support at least vulkan 1.2
            if(!(devProp.apiVersion & VK_API_VERSION_1_2))
                continue;

            //Ignore any device that is iGPU or CPU
            if(devProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU || devProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
                continue;

            LOG_F(INFO, "Device selected: %s", devProp.deviceName);
            mPhysDevice = device;
            return;
        }

        

        LOG_F(ERROR, "No compatible device was found!");
        throw Exception();

    }

    void GraphicsCore::PickPhysicalDevice(uint32_t devIndex)
    {
        //Count all avaliable devices
        uint32_t devCount = 0;
        vkEnumeratePhysicalDevices(mInstance, &devCount, nullptr);

        //If there isn't at least one device throw exception
        if(devCount < 1)
        {
            LOG_F(ERROR, "There are no avaliable devices on your system!");
            throw Exception();
        }

        //Check if device with provided index even exists
        if(devCount <= devIndex)
        {
            //If preffered device does not exist fall back to auto picking GPU
            LOG_F(ERROR, "Preffered device is not avaliable! Falling back to auto pick...");
            AutoPickPhysicalDevice();
            return;
        }

        //Obtain all devices
        std::vector<VkPhysicalDevice> vecDevices(devCount);
        vkEnumeratePhysicalDevices(mInstance, &devCount, &vecDevices[0]);

        VkPhysicalDeviceProperties devProp;
        vkGetPhysicalDeviceProperties(vecDevices[devIndex], &devProp);

        //Check if device supports at least Vulkan 1.2
        if(!(devProp.apiVersion & VK_API_VERSION_1_2))
        {
            //If not fall back to auto pick
            LOG_F(ERROR, "Your preffered device does not support Vulkan 1.2! Falling back to auto pick...");
            AutoPickPhysicalDevice();
            return;
        }

        LOG_F(INFO, "Device selected: %s", devProp.deviceName);
        mPhysDevice = vecDevices[devIndex];
        return;
    }

    bool GraphicsCore::CheckValidationSupport()
    {
        uint32_t layerCount = 0;
        std::vector<VkLayerProperties> layerData;

        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        layerData.resize(layerCount);

        vkEnumerateInstanceLayerProperties(&layerCount, &layerData[0]);
        
        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProp : layerData)
            {
                if (strcmp(layerName, layerProp.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
                return false;
        }

        return true;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL GraphicsCore::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LOG_F(INFO, "%s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG_F(INFO, "%s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_F(WARNING, "%s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG_F(ERROR, "%s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
            LOG_F(ERROR, "%s", pCallbackData->pMessage);
            break;
        default:
            LOG_F(INFO, "%s", pCallbackData->pMessage);
            break;
        }

        return VK_FALSE;
    }

    void GraphicsCore::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
    }

    void GraphicsCore::SetupDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		VkResult res = CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger);
		VK_THROW_IF_FAILED(res);

		LOG_F(INFO, "Debug messenger set up!");
	}

}