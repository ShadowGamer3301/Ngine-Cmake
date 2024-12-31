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
    class NGAPI GraphicsCore
    {
    public:
        GraphicsCore(NgineWindow* pWindow);
        ~GraphicsCore();

        void RenderFrame();


    private:
        void CreateFactory();
        void SelectAdapter(int prefferedAdapter);
        void AutoSelectAdapter();
        void CreateDevice();
        void CreateSwapchain(NgineWindow* pWindow);
        void CreateRenderTargetView();
        uint32_t GenerateExclusiveShaderId();
        uint32_t GenerateExclusiveModelId();
        uint32_t GenerateExclusiveTextureId();
        void CreateViewport(NgineWindow* pWindow);
        void CreateRasterizer();
        void CreateDepthStencilState(NgineWindow* pWindow);
        void CreateDepthStencilBuffer(NgineWindow* pWindow);
        void CreateDepthStencilView();
        void CreateLinearSampler();

    private:
        IDXGIFactory* pFactory = nullptr;
        IDXGISwapChain* pSwapchain = nullptr;
        IDXGIAdapter* pAdapter = nullptr;
        ID3D11Device* pDevice = nullptr;
        ID3D11DeviceContext* pContext = nullptr;
        ID3D11RenderTargetView* pRenderTarget = nullptr;
        ID3D11Texture2D* pBackBuffer = nullptr;
        D3D11_VIEWPORT mViewport = {};
        ID3D11RasterizerState* pRasterizer = nullptr;
        ID3D11Texture2D* pDepthBuffer = nullptr;
        ID3D11DepthStencilState* pDepthState = nullptr;
        ID3D11DepthStencilView* pDepthView = nullptr;
        ID3D11SamplerState* pLinearSampler = nullptr;
    };

#elif defined(TARGET_PLATFORM_XBOX)
    class NGAPI GraphicsCore
    {
    public:
        GraphicsCore(NgineWindow* pWindow);
        ~GraphicsCore();

        void RenderFrame();

    private:
        void CreateFactory();
        void SelectAdapter(int prefferedAdapter);
        void AutoSelectAdapter();
        void CreateDevice();
        void CreateSwapchain(NgineWindow* pWindow);
        void CreateRenderTargetView();
        uint32_t GenerateExclusiveShaderId();
        uint32_t GenerateExclusiveModelId();
        uint32_t GenerateExclusiveTextureId();
        void CreateViewport(NgineWindow* pWindow);
        void CreateRasterizer();
        void CreateDepthStencilState(NgineWindow* pWindow);
        void CreateDepthStencilBuffer(NgineWindow* pWindow);
        void CreateDepthStencilView();
        void CreateLinearSampler();

    private:
        IDXGIFactory* pFactory = nullptr;
        IDXGISwapChain* pSwapchain = nullptr;
        IDXGIAdapter* pAdapter = nullptr;
        ID3D11Device* pDevice = nullptr;
        ID3D11DeviceContext* pContext = nullptr;
        ID3D11RenderTargetView* pRenderTarget = nullptr;
        ID3D11Texture2D* pBackBuffer = nullptr;
        D3D11_VIEWPORT mViewport = {};
        ID3D11RasterizerState* pRasterizer = nullptr;
        ID3D11Texture2D* pDepthBuffer = nullptr;
        ID3D11DepthStencilState* pDepthState = nullptr;
        ID3D11DepthStencilView* pDepthView = nullptr;
        ID3D11SamplerState* pLinearSampler = nullptr;
    };
#endif
}