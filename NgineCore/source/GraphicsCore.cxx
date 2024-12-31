#include "GraphicsCore.h"
#include "Core.hxx"
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
    }

    GraphicsCore::~GraphicsCore()
    {
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
    desc.Windowed = true;
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

#endif
}