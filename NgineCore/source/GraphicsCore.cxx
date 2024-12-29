#include "GraphicsCore.h"
#include "Exception.h"
#include "FileUtils.h"

namespace Ngine
{
#if defined(TARGET_PLATFORM_LINUX)

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        
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


    GraphicsCore::GraphicsCore(NgineWindow* pWindow)
    {
        CreateInstance();

        bool devAutoPick = FileUtils::GetBoolFromConfig("Resource/ngine.ini", "General", "AutoPickDevice");
        int devIndex = FileUtils::GetIntegerFromConfig("Resource/ngine.ini", "General", "ManualDeviceIndex");
        
        CreateSurface(pWindow);
        
        if(devAutoPick)
            AutoPickPhysicalDevice();
        else
            PickPhysicalDevice(devIndex);

        ObtainQueueIndexes();
        CreateLogicDevice();
        CreateSwapchain(pWindow);
        CreateImageViews();
        CreateRenderPass();
        CreateFrameBuffers();
        CreateCommandPool();
        CreateCommandBuffer();
        CreateSyncObjects();
    }

    GraphicsCore::~GraphicsCore()
    {
        vkDeviceWaitIdle(mDevice);

        for (size_t i = 0; i < vecModels.size(); i++)
        {
            for (size_t j = 0; j < vecModels[i].vecMeshes.size(); j++)
            {
                vkFreeMemory(mDevice, vecModels[i].vecMeshes[j].mVertexMemory, nullptr);
                vkDestroyBuffer(mDevice, vecModels[i].vecMeshes[j].mVertexBuffer, nullptr);
                vkFreeMemory(mDevice, vecModels[i].vecMeshes[j].mIndexMemory, nullptr);
                vkDestroyBuffer(mDevice, vecModels[i].vecMeshes[j].mIndexBuffer, nullptr);
            }

            for (size_t j = 0; j < vecModels[i].vecUniformBuffers.size(); j++)
            {
                vkDestroyBuffer(mDevice, vecModels[i].vecUniformBuffers[j], nullptr);
                vkFreeMemory(mDevice, vecModels[i].vecUniformMemory[j], nullptr);

            }
        }


        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(mDevice, vecImgAvSemaphores[i], nullptr);
            vkDestroySemaphore(mDevice, vecRenderFinsihSemaphores[i], nullptr);
            vkDestroyFence(mDevice, vecFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(mDevice, mCmdPool, nullptr);
        for (auto framebuffer : vecFrameBuffers)
            vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
	    vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
        for (auto imageView : vecSwapImageViews)
		    vkDestroyImageView(mDevice, imageView, nullptr);
        vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
        vkDestroyDevice(mDevice, nullptr);
        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
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
        extensions.push_back("VK_KHR_xlib_surface"); //Add X11 extension manually since it won't be added automatically by glfw funcitons

        if (enableVL) //If validation layers are enabled add debug extension
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

        LOG_F(INFO, "Number of detected devices: %d", devCount);

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

    void GraphicsCore::CreateSurface(NgineWindow* pWindow)
    {
        VkXlibSurfaceCreateInfoKHR surfInfo = {};
        surfInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        surfInfo.window = pWindow->GetX11Window();
        surfInfo.dpy = pWindow->GetX11Display();

        VkResult res = vkCreateXlibSurfaceKHR(mInstance, &surfInfo, nullptr, &mSurface);
        VK_THROW_IF_FAILED(res);

        LOG_F(INFO, "Vulkan surface created (XLIB/X11 Mode)");
    }

    void GraphicsCore::ObtainQueueIndexes()
    {
        uint32_t queueCount = 0;
        std::vector<VkQueueFamilyProperties> vecQueueProps;

        vkGetPhysicalDeviceQueueFamilyProperties(mPhysDevice, &queueCount, nullptr);
        vecQueueProps.resize(queueCount);

        vkGetPhysicalDeviceQueueFamilyProperties(mPhysDevice, &queueCount, &vecQueueProps[0]);

        int i = 0;
	    VkBool32 presentSupport = false;

        for(const auto& queueFamily : vecQueueProps)
        {
            if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                mQueueData.mGraphicsQueueIndex = i;

            vkGetPhysicalDeviceSurfaceSupportKHR(mPhysDevice, i, mSurface, &presentSupport);

            if(presentSupport)
                mQueueData.mPresentationQueueIndex = i;

            i++;
        }
    }

    void GraphicsCore::CreateLogicDevice()
    {
        bool enableVL = FileUtils::GetBoolFromConfig("Resource/ngine.ini", "General", "EnableGfxDebugMode");
        
        if(!mQueueData.mGraphicsQueueIndex.has_value() || !mQueueData.mPresentationQueueIndex.has_value())
        {
            LOG_F(ERROR, "Queue family indexes are missing...");
            throw Exception();
        }

        float priority = 1.0f;
        VkDeviceQueueCreateInfo graphicsQueueInfo = {};
        graphicsQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphicsQueueInfo.pQueuePriorities = &priority;
        graphicsQueueInfo.queueCount = 1;
        graphicsQueueInfo.queueFamilyIndex = mQueueData.mGraphicsQueueIndex.value();

        VkDeviceQueueCreateInfo presentationQueueInfo = {};
        presentationQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        presentationQueueInfo.pQueuePriorities = &priority;
        presentationQueueInfo.queueCount = 1;
        presentationQueueInfo.queueFamilyIndex = mQueueData.mPresentationQueueIndex.value();

        VkDeviceQueueCreateInfo queueArray[] = { graphicsQueueInfo, presentationQueueInfo };

        VkPhysicalDeviceFeatures devFeatures = {};

        VkDeviceCreateInfo devInfo = {};
        devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        devInfo.pQueueCreateInfos = queueArray;
        devInfo.queueCreateInfoCount = sizeof(queueArray) / sizeof(VkDeviceQueueCreateInfo);
        devInfo.pEnabledFeatures = &devFeatures;
        devInfo.enabledExtensionCount = deviceExtensions.size();
        devInfo.ppEnabledExtensionNames = deviceExtensions.data();
        
        if (enableVL != 0)
        {
            devInfo.enabledLayerCount = validationLayers.size();
            devInfo.ppEnabledLayerNames = validationLayers.data();
        }

        VkResult res = vkCreateDevice(mPhysDevice, &devInfo, nullptr, &mDevice);
        VK_THROW_IF_FAILED(res);
        
        vkGetDeviceQueue(mDevice, mQueueData.mGraphicsQueueIndex.value(), 0, &mGraphicsQueue);
        vkGetDeviceQueue(mDevice, mQueueData.mPresentationQueueIndex.value(), 0, &mPresentationQueue);
    }

    void GraphicsCore::CreateSwapchain(NgineWindow* pWindow)
    {
        const auto avaliableFormats = ObtainSurfaceFormats();
	    const auto avaliableModes = ObtainSurfaceModes();

        VkSurfaceFormatKHR surfFormat = ChooseSurfaceFormat(avaliableFormats);
	    VkPresentModeKHR presentMode = ChoosePresentMode(avaliableModes);
	    VkExtent2D extent = ChooseSwapExtent(pWindow);

        VkSurfaceCapabilitiesKHR caps = {};
	    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysDevice, mSurface, &caps);

	    uint32_t imageCount = caps.minImageCount + 1;
        if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
		imageCount = caps.maxImageCount;

        VkSwapchainCreateInfoKHR swapInfo = {};
        swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapInfo.surface = mSurface;
        swapInfo.imageFormat = surfFormat.format;
        swapInfo.imageColorSpace = surfFormat.colorSpace;
        swapInfo.imageExtent = extent;
        swapInfo.imageArrayLayers = 1;
        swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapInfo.minImageCount = imageCount;
        swapInfo.preTransform = caps.currentTransform;
        swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapInfo.presentMode = presentMode;
        swapInfo.clipped = VK_TRUE;
        swapInfo.oldSwapchain = VK_NULL_HANDLE;

        uint32_t queueFamilyIndices[] = { mQueueData.mGraphicsQueueIndex.value(), mQueueData.mPresentationQueueIndex.value() };

        if (queueFamilyIndices[0] != queueFamilyIndices[1])
        {
            swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapInfo.queueFamilyIndexCount = 2;
            swapInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapInfo.queueFamilyIndexCount = 0;
            swapInfo.pQueueFamilyIndices = nullptr;
        }

        VkResult res = vkCreateSwapchainKHR(mDevice, &swapInfo, nullptr, &mSwapchain);
        VK_THROW_IF_FAILED(res);

        vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
        vecSwapImages.resize(imageCount);
        vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, &vecSwapImages[0]);

        mSwapFormat = swapInfo.imageFormat;
        mSwapExtent = extent;
    }

    std::vector<VkSurfaceFormatKHR> GraphicsCore::ObtainSurfaceFormats()
    {
        uint32_t formatCount = 0;
        std::vector<VkSurfaceFormatKHR> formatData;
        vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysDevice, mSurface, &formatCount, nullptr);

        if (formatCount == 0)
            throw Exception();
        
        formatData.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysDevice, mSurface, &formatCount, &formatData[0]);

        return formatData;
    }

    std::vector<VkPresentModeKHR> GraphicsCore::ObtainSurfaceModes()
    {
        uint32_t modeCount = 0;
        std::vector<VkPresentModeKHR> modeData;
        vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysDevice, mSurface, &modeCount, nullptr);

        if (modeCount == 0)
            throw Exception();
        
        modeData.resize(modeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysDevice, mSurface, &modeCount, &modeData[0]);

        return modeData;
    }

    VkSurfaceFormatKHR GraphicsCore::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avaliableFormats)
    {
        for (const auto& format : avaliableFormats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return format;
        }

        return avaliableFormats[0];
    }

    VkPresentModeKHR GraphicsCore::ChoosePresentMode(const std::vector<VkPresentModeKHR>& avaliableModes)
    {
        for (const auto& mode : avaliableModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                return mode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D GraphicsCore::ChooseSwapExtent(NgineWindow* pWindow)
    {
        VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysDevice, mSurface, &capabilities);

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else
	{
		int width = pWindow->GetWidth(), height = pWindow->GetHeight();

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    void GraphicsCore::CreateImageViews()
    {
        vecSwapImageViews.resize(vecSwapImages.size());

        for (size_t i = 0; i < vecSwapImages.size(); i++)
        {
            VkImageViewCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageInfo.image = vecSwapImages.at(i);
            imageInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageInfo.format = mSwapFormat;
            imageInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageInfo.subresourceRange.baseMipLevel = 0;
            imageInfo.subresourceRange.levelCount = 1;
            imageInfo.subresourceRange.baseArrayLayer = 0;
            imageInfo.subresourceRange.layerCount = 1;

            VkResult res = vkCreateImageView(mDevice, &imageInfo, nullptr, &vecSwapImageViews[i]);
            VK_THROW_IF_FAILED(res);
        }
    }

    void GraphicsCore::CreatePipeline(Shader& shader)
    {

        std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        auto bindingDesc = Vertex::GetBindingDescription();
        auto attrDesc = Vertex::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
        vertexInputInfo.vertexAttributeDescriptionCount = attrDesc.size();
        vertexInputInfo.pVertexAttributeDescriptions = attrDesc.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)mSwapExtent.width;
        viewport.height = (float)mSwapExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = mSwapExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &shader.mDescLayout; 
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        VkResult res = vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &shader.mPipelineLayout);
        VK_THROW_IF_FAILED(res);

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shader.mShaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = shader.mPipelineLayout;
        pipelineInfo.renderPass = mRenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        res = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &shader.mPipeline);
        VK_THROW_IF_FAILED(res);
    }

    void GraphicsCore::CreateRenderPass()
    {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = mSwapFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkResult res = vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass);
        VK_THROW_IF_FAILED(res);
    }

    void GraphicsCore::CreateFrameBuffers()
    {
        vecFrameBuffers.resize(vecSwapImageViews.size());

        for (size_t i = 0; i < vecSwapImageViews.size(); i++)
        {
            VkImageView attachments[] = {vecSwapImageViews.at(i)};
            VkFramebufferCreateInfo fbInfo = {};
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.renderPass = mRenderPass;
            fbInfo.attachmentCount = 1;
            fbInfo.pAttachments = attachments;
            fbInfo.width = mSwapExtent.width;
            fbInfo.height = mSwapExtent.height;
            fbInfo.layers = 1;

            VkResult res = vkCreateFramebuffer(mDevice, &fbInfo, nullptr, &vecFrameBuffers[i]);
            VK_THROW_IF_FAILED(res);
        }
    }

    void GraphicsCore::CreateCommandPool()
    {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        cmdPoolInfo.queueFamilyIndex = mQueueData.mGraphicsQueueIndex.value();

        VkResult res = vkCreateCommandPool(mDevice, &cmdPoolInfo, nullptr, &mCmdPool);
        VK_THROW_IF_FAILED(res);
    }

    void GraphicsCore::CreateCommandBuffer()
    {
        vecCmdBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = mCmdPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = vecCmdBuffers.size();

        VkResult res = vkAllocateCommandBuffers(mDevice, &allocInfo, vecCmdBuffers.data());
        VK_THROW_IF_FAILED(res);
    }

    void GraphicsCore::RecordCommandBuffer(VkCommandBuffer cmdBuffer, uint32_t imgIndex)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        VkResult res = vkBeginCommandBuffer(cmdBuffer, &beginInfo);
        VK_THROW_IF_FAILED(res);
    }

    void GraphicsCore::CreateSyncObjects()
    {
        vecImgAvSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        vecRenderFinsihSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        vecFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkResult res = vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &vecImgAvSemaphores[i]);
            VK_THROW_IF_FAILED(res);

            res = vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &vecRenderFinsihSemaphores[i]);
            VK_THROW_IF_FAILED(res);

            res = vkCreateFence(mDevice, &fenceInfo, nullptr, &vecFlightFences[i]);
            VK_THROW_IF_FAILED(res);
        }
    }

    void GraphicsCore::RecreateSwapChain(NgineWindow* p)
    {
        int width = p->GetWidth(); int height = p->GetHeight();
        
        vkDeviceWaitIdle(mDevice);

        for (size_t i = 0; i < vecFrameBuffers.size(); i++)
        {
            vkDestroyFramebuffer(mDevice, vecFrameBuffers[i], nullptr);
        }

        for (size_t i = 0; i < vecSwapImageViews.size(); i++)
        {
            vkDestroyImageView(mDevice, vecSwapImageViews[i], nullptr);
        }

        vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);

        CreateSwapchain(p);
        CreateImageViews();
        CreateFrameBuffers();
    }

    void GraphicsCore::CreateVertexBuffer(Mesh& m, std::vector<Vertex> verts)
    {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;

        //Callucate memory size required to hold the buffer
        VkDeviceSize bufferSize = sizeof(verts[0]) * verts.size();

        //Create buffer that can interact with CPU
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

        //Copy data from CPU to staging buffer
        void* data;
        vkMapMemory(mDevice, stagingMemory, 0, bufferSize, 0, &data);
        memcpy(data, verts.data(), (size_t)bufferSize);
        vkUnmapMemory(mDevice, stagingMemory);

        //Create vertex buffer that will be utilized by GPU
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m.mVertexBuffer, m.mVertexMemory);

        //Copy data from staging buffer to vertex buffer
        CopyBuffer(stagingBuffer, m.mVertexBuffer, bufferSize);

        //Clear staging buffer
        vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
        vkFreeMemory(mDevice, stagingMemory, nullptr);
    }

    uint32_t GraphicsCore::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props)
    {
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(mPhysDevice, &memProps);

        for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & props) == props)
                return i;
        }

        throw Exception();
    }

    void GraphicsCore::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer& buffer, VkDeviceMemory& memory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult res = vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer);
        VK_THROW_IF_FAILED(res);

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(mDevice, buffer, &memReq);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memReq.memoryTypeBits, props);

        res = vkAllocateMemory(mDevice, &allocInfo, nullptr, &memory);
        VK_THROW_IF_FAILED(res);

        vkBindBufferMemory(mDevice, buffer, memory, 0);
    }

    void GraphicsCore::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = mCmdPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmdBuffer;
        vkAllocateCommandBuffers(mDevice, &allocInfo, &cmdBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmdBuffer, &beginInfo);

        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;

        vkCmdCopyBuffer(cmdBuffer, src, dst, 1, &copyRegion);
        vkEndCommandBuffer(cmdBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

        vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(mGraphicsQueue);

        vkFreeCommandBuffers(mDevice, mCmdPool, 1, &cmdBuffer);
    }

    void GraphicsCore::CreateIndexBuffer(Mesh& m, std::vector<uint16_t> indices)
    {
        //Calculate memory required to hold the buffer
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;

        //Create staging buffer
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

        //Copy data to staging buffer
        void* data;
        vkMapMemory(mDevice, stagingMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t)bufferSize);
        vkUnmapMemory(mDevice, stagingMemory);

        //Create index buffer
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m.mIndexBuffer, m.mIndexMemory);
        
        //Copy data from staging to index buffer
        CopyBuffer(stagingBuffer, m.mIndexBuffer, bufferSize);

        //Release staging buffer
        vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
        vkFreeMemory(mDevice, stagingMemory, nullptr);
    }

    void GraphicsCore::CreateDescriptorSetLayout(Shader& shader)
    {
        VkDescriptorSetLayoutBinding mvpLayoutBinding = {};
        mvpLayoutBinding.binding = 0;
        mvpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mvpLayoutBinding.descriptorCount = 1;
        mvpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        mvpLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &mvpLayoutBinding;

        VkResult res = vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &shader.mDescLayout);
        VK_THROW_IF_FAILED(res);
    }

    void GraphicsCore::CreateMvpBuffer(Model& m)
    {
        VkDeviceSize bufferSize = sizeof(MVP);
	
        m.vecUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m.vecUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
        m.vecUniformMemory.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m.vecUniformBuffers[i], m.vecUniformMemory[i]);
            vkMapMemory(mDevice, m.vecUniformMemory[i], 0, bufferSize, 0, &m.vecUniformBuffersMapped[i]);
        }
    }

    void GraphicsCore::CreateDescriptorPool(Shader& shader)
    {
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkResult res = vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &shader.mDescPool);
        VK_THROW_IF_FAILED(res);
    }

    void GraphicsCore::CreateDescriptorSets(Model& m, Shader& shader)
    {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, shader.mDescLayout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = shader.mDescPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        for (auto& mesh : m.vecMeshes)
        {
            mesh.vecDescSets.resize(MAX_FRAMES_IN_FLIGHT);
            VkResult res = vkAllocateDescriptorSets(mDevice, &allocInfo, mesh.vecDescSets.data());
            VK_THROW_IF_FAILED(res);
        }
        

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = m.vecUniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MVP);

            for (auto& mesh : m.vecMeshes)
            {
                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = mesh.vecDescSets[i];
                descriptorWrite.dstBinding = 0;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;
                descriptorWrite.pImageInfo = nullptr;
                descriptorWrite.pTexelBufferView = nullptr;

                vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
            }
        }

    }

    void GraphicsCore::DrawFrame(NgineWindow* pWin)
    {
        uint32_t bindCount = 0;

        vkWaitForFences(mDevice, 1, &vecFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imgIndex;
        VkResult res = vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, vecImgAvSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imgIndex);
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || mFramebufferResized) {
            mFramebufferResized = false;
            RecreateSwapChain(pWin);
            return;
        }
        else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
            throw VulkanException(res);

        vkResetFences(mDevice, 1, &vecFlightFences[mCurrentFrame]);
        vkResetCommandBuffer(vecCmdBuffers[mCurrentFrame], 0);
        RecordCommandBuffer(vecCmdBuffers[mCurrentFrame], imgIndex);

        VkClearValue clearColor = { 0.0f, 0.4f, 0.6f, 1.0f };

        VkRenderPassBeginInfo rpInfo = {};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpInfo.renderPass = mRenderPass;
        rpInfo.framebuffer = vecFrameBuffers.at(imgIndex);
        rpInfo.renderArea.offset = { 0,0 };
        rpInfo.renderArea.extent = mSwapExtent;
        rpInfo.clearValueCount = 1;
        rpInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(vecCmdBuffers[mCurrentFrame], &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)mSwapExtent.width;
        viewport.height = (float)mSwapExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(vecCmdBuffers[mCurrentFrame], 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset = { 0,0 };
        scissor.extent = mSwapExtent;
        vkCmdSetScissor(vecCmdBuffers[mCurrentFrame], 0, 1, &scissor);

        vkCmdEndRenderPass(vecCmdBuffers[mCurrentFrame]);

	    res = vkEndCommandBuffer(vecCmdBuffers[mCurrentFrame]);
        VK_THROW_IF_FAILED(res);

        VkSemaphore waitSemaphores[] = { vecImgAvSemaphores[mCurrentFrame]};
        VkSemaphore signalSemaphores[] = { vecRenderFinsihSemaphores[mCurrentFrame]};
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vecCmdBuffers[mCurrentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        res = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, vecFlightFences[mCurrentFrame]);
        VK_THROW_IF_FAILED(res);

        VkSwapchainKHR swapchains[] = {mSwapchain};

        VkPresentInfoKHR presInfo = {};
        presInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presInfo.waitSemaphoreCount = 1;
        presInfo.pWaitSemaphores = signalSemaphores;
        presInfo.swapchainCount = 1;
        presInfo.pSwapchains = swapchains;
        presInfo.pImageIndices = &imgIndex;
        presInfo.pResults = nullptr;

        res = vkQueuePresentKHR(mPresentationQueue, &presInfo);

        if (res == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapChain(pWin);
            return;
        }
        else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
            throw VulkanException(res);

        
	    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    std::array<VkVertexInputAttributeDescription, 3> Vertex::GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attrDesc = {};

        //Position
        attrDesc[0].binding = 0;
        attrDesc[0].location = 0;
        attrDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrDesc[0].offset = offsetof(Vertex, pos);

        //Color
        attrDesc[1].binding = 0;
        attrDesc[1].location = 1;
        attrDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrDesc[1].offset = offsetof(Vertex, color);

        //UV
        attrDesc[2].binding = 0;
        attrDesc[2].location = 2;
        attrDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
        attrDesc[2].offset = offsetof(Vertex, uv);

        return attrDesc;
    }

    VkVertexInputBindingDescription Vertex::GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDesc = {};
        bindingDesc.binding = 0;
        bindingDesc.stride = sizeof(Vertex);
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDesc;
    }



#elif defined(TARGET_PLATFORM_WINDOWS)

    GraphicsCore::GraphicsCore(NgineWindow* pWindow)
    {
        bool devAuto = FileUtils::GetBoolFromConfig("Resource/ngine.ini", "General", "AutoPickDevice");
        int devIndex = FileUtils::GetIntegerFromConfig("Resource/ngine.ini", "General", "ManualDeviceIndex");

        LOG_F(INFO, "Initializing DirectX 11 API...");
        CreateFactory();
        if (devAuto)
            AutoSelectAdapter();
        else
            SelectAdapter(devIndex);
        CreateDevice();
        CreateSwapchain(pWindow);
        CreateRasterizer();
        CreateDepthStencilBuffer(pWindow);
        CreateDepthStencilState(pWindow);
        CreateDepthStencilView();
        CreateRenderTargetView();
        CreateViewport(pWindow);
        LOG_F(INFO, "DirectX 11 initalized!");
        LOG_F(INFO, "Loading engine assets...");
    }

    GraphicsCore::~GraphicsCore()
    {
        DX_COM_RELEASE(pRasterizer);
        DX_COM_RELEASE(pSwapchain);
        DX_COM_RELEASE(pContext);
        DX_COM_RELEASE(pDevice);
        DX_COM_RELEASE(pAdapter);
        DX_COM_RELEASE(pFactory);
    }

    void GraphicsCore::RenderFrame()
    {
        float color[4] = { 0.0f, 0.2f, 0.6f, 1.0f };

        pContext->ClearRenderTargetView(pRenderTarget, color);

        pSwapchain->Present(0, 0);
    }

    void GraphicsCore::CreateFactory()
    {
        HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (LPVOID*)&pFactory);
        DX_THROW_IF_FAILED(hr);
        LOG_F(INFO, "DXGI Factory Created!");
    }

    void GraphicsCore::AutoSelectAdapter()
    {
        UINT i = 0;
        
        while (pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC desc = {};
            pAdapter->GetDesc(&desc);
            LOG_F(INFO, "Testind device no.%d: %ls", i, desc.Description);

            if (desc.DedicatedVideoMemory > 0)
            {
                LOG_F(INFO, "Adapter found! Device name: %ls", desc.Description);
                return;
            }

            DX_COM_RELEASE(pAdapter);
            i++;
        }

        LOG_F(ERROR, "Cannot find any suitable adapters!");
        throw DirectXException(DXGI_ERROR_NOT_FOUND);
    }

    void GraphicsCore::SelectAdapter(int devIndex)
    {
        if (pFactory->EnumAdapters(devIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC desc = {};
            pAdapter->GetDesc(&desc);
            LOG_F(INFO, "Testind device with id %d: %ls", devIndex, desc.Description);

            if (desc.DedicatedVideoMemory > 0)
            {
                LOG_F(INFO, "Adapter found! Device name: %ls", desc.Description);
                return;
            }
        }

        DX_COM_RELEASE(pAdapter);

        LOG_F(ERROR, "Preffered adapter is either not compatible or unavaliable");
        LOG_F(WARNING, "Falling back to AutoSelectAdapter()");
        AutoSelectAdapter();
    }

    void GraphicsCore::CreateDevice()
    {
        bool enableDbg = FileUtils::GetBoolFromConfig("Resource/ngine.ini", "General", "EnableGfxDebugMode");

        DWORD flag = 0;

        if (enableDbg)
            flag = D3D11_CREATE_DEVICE_DEBUG;

        D3D_FEATURE_LEVEL flevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };

        HRESULT hr = D3D11CreateDevice(pAdapter, D3D_DRIVER_TYPE_UNKNOWN, 0, flag, flevels, _countof(flevels), D3D11_SDK_VERSION, &pDevice, nullptr, &pContext);
        
        DX_THROW_IF_FAILED(hr);
        LOG_F(INFO, "D3D11 device and context created!");
    }

    void GraphicsCore::CreateSwapchain(NgineWindow* pWindow)
    {
        DXGI_SWAP_CHAIN_DESC desc = {};
        desc.BufferCount = 1;
        desc.BufferDesc.Width = pWindow->GetWidth();
        desc.BufferDesc.Height = pWindow->GetHeight();
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Windowed = !pWindow->IsFullscreen();
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.OutputWindow = pWindow->GetWindowHandle();
        desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        HRESULT hr = pFactory->CreateSwapChain(pDevice, &desc, &pSwapchain);
        DX_THROW_IF_FAILED(hr);

        LOG_F(INFO, "DXGI swapchain created!");
    }

    void GraphicsCore::CreateRasterizer()
    {
        D3D11_RASTERIZER_DESC desc = {};
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_BACK;
        desc.FrontCounterClockwise = FALSE;
        desc.DepthBias = 0;
        desc.DepthBiasClamp = 0.0f;
        desc.SlopeScaledDepthBias = 0.0f;
        desc.DepthClipEnable = FALSE;
        desc.ScissorEnable = FALSE;
        desc.MultisampleEnable = FALSE;
        desc.AntialiasedLineEnable = FALSE;

        HRESULT hr = pDevice->CreateRasterizerState(&desc, &pRasterizer);
        DX_THROW_IF_FAILED(hr);

        LOG_F(INFO, "Rasterizer state created!");
        pContext->RSSetState(pRasterizer);
    }

    void GraphicsCore::CreateLinearSampler()
    {
        D3D11_SAMPLER_DESC desc = {};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        desc.MinLOD = 0;
        desc.MaxLOD = D3D11_FLOAT32_MAX;

        DX_THROW_IF_FAILED(pDevice->CreateSamplerState(&desc, &pLinearSampler));
        LOG_F(INFO, "Linear sampler state created!");
    }

    void GraphicsCore::CreateDepthStencilState(NgineWindow* pWindow)
    {
        D3D11_DEPTH_STENCIL_DESC desc = {};
        //Depth test parameters
        desc.DepthEnable = true;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_LESS;
        //Stencil test parameters
        desc.StencilEnable = true;
        desc.StencilReadMask = 0xFF;
        desc.StencilWriteMask = 0xFF;
        //If pixel is front facing
        desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        //If pixel is back facing
        desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        DX_THROW_IF_FAILED(pDevice->CreateDepthStencilState(&desc, &pDepthState));
        pContext->OMSetDepthStencilState(pDepthState, 0);

        LOG_F(INFO, "Depth/stencil state created and set!");
    }

    void GraphicsCore::CreateDepthStencilBuffer(NgineWindow* pWindow)
    {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = pWindow->GetWidth();
        desc.Height = pWindow->GetHeight();
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        DX_THROW_IF_FAILED(pDevice->CreateTexture2D(&desc, nullptr, &pDepthBuffer));
        LOG_F(INFO, "Depth/stencil buffer created!");

    }

    void GraphicsCore::CreateDepthStencilView()
    {

        D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.Flags = 0;
        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;

        DX_THROW_IF_FAILED(pDevice->CreateDepthStencilView(pDepthBuffer, &desc, &pDepthView));
        LOG_F(INFO, "Depth/stencil view created!");
    }

    void GraphicsCore::CreateRenderTargetView()
    {
        DX_THROW_IF_FAILED(pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));
        LOG_F(INFO, "Buffer obtained from swapchain");

        //Create Render target view from obtained buffer
        DX_THROW_IF_FAILED(pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTarget));
        LOG_F(INFO, "D3D11 render target view created");

        pContext->OMSetRenderTargets(1, &pRenderTarget, pDepthView);
        LOG_F(INFO, "Render target view and depth stencil view set!");
    }

    void GraphicsCore::CreateViewport(NgineWindow* pWindow)
    {
        mViewport.TopLeftX = 0.0f;
        mViewport.TopLeftY = 0.0f;
        mViewport.Width = (float)pWindow->GetWidth();
        mViewport.Height = (float)pWindow->GetHeight();
        mViewport.MinDepth = 0.0f;
        mViewport.MaxDepth = 1.0f;

        pContext->RSSetViewports(1, &mViewport);
        LOG_F(INFO, "Viewport set!");
    }

#elif defined(TARGET_PLATFORM_XBOX)

    GraphicsCore::GraphicsCore(Window* pWindow)
    {

    }

    GraphicsCore::~GraphicsCore()
    {

    }

#endif
}