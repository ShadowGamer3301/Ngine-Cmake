#pragma once
#include "Core.hxx"
#include "Window.h"
#include "Exception.h"

namespace Ngine
{
#if defined(TARGET_PLATFORM_LINUX)

    struct Vertex
    {
        glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv;

        static VkVertexInputBindingDescription GetBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();
    };

    struct MVP
    {
        glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
    };

    class Shader
    {
        friend class GraphicsCore;
    public:
        inline uint32_t GetShaderId() const noexcept { return mId; }

    private:
        uint32_t mId = 0;
		VkShaderModule mVertex;
		VkShaderModule mFragment;
		VkPipelineShaderStageCreateInfo mShaderStages[2];
		VkPipelineLayout mPipelineLayout;
		VkPipeline mPipeline;
		VkDescriptorSetLayout mDescLayout;
		VkDescriptorPool mDescPool;
    };

    class Mesh
    {
        friend class GraphicsCore;
	private:
		uint32_t mVertexCount = 0;
		uint32_t mIndexCount = 0;
		VkBuffer mVertexBuffer;
		VkBuffer mIndexBuffer;
		VkDeviceMemory mVertexMemory;
		VkDeviceMemory mIndexMemory;
		std::vector<VkDescriptorSet> vecDescSets;
    };

    class Model
    {
	    friend class GraphicsCore;
		friend class GameObject3D;
	private:
		std::vector<Mesh> vecMeshes;
		std::vector<VkBuffer> vecUniformBuffers;
		std::vector<VkDeviceMemory> vecUniformMemory;
		std::vector<void*> vecUniformBuffersMapped;
    };

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

        void DrawFrame(NgineWindow* pWin);

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
        void CreateLogicDevice();
        void CreateSwapchain(NgineWindow* pWindow);
        std::vector<VkSurfaceFormatKHR> ObtainSurfaceFormats();
		std::vector<VkPresentModeKHR> ObtainSurfaceModes();
		VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avaliableFormats);
		VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& avaliableModes);
		VkExtent2D ChooseSwapExtent(NgineWindow* pWindow);
        void CreateImageViews();
        void CreatePipeline(Shader& shader);
        void CreateRenderPass();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateCommandBuffer();
		void RecordCommandBuffer(VkCommandBuffer cmdBuffer, uint32_t imgIndex);
		void CreateSyncObjects();
        void RecreateSwapChain(NgineWindow* p);
		void CreateVertexBuffer(Mesh& m, std::vector<Vertex> verts);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer& buffer, VkDeviceMemory& memory);
		void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
		void CreateIndexBuffer(Mesh& m, std::vector<uint16_t> indices);
		void CreateDescriptorSetLayout(Shader& shader);
		void CreateMvpBuffer(Model& m);
		//void UpdateMvpBuffer(uint32_t frameIndex, GameObject3D& go);
		void CreateDescriptorPool(Shader& shader);
		void CreateDescriptorSets(Model& m, Shader& shader);

    private:
		std::vector<Shader> vecShaders;
		std::vector<Model> vecModels;
		uint32_t mCurrentFrame = 0;
		bool mFramebufferResized = false;
		bool mPauseOnMimimize = false;
		std::optional<uint32_t> mUsedShader;
		glm::mat4 view;
		glm::mat4 proj;

    private:
        VkInstance mInstance;
        VkPhysicalDevice mPhysDevice;
        VkDevice mDevice;
        VkDebugUtilsMessengerEXT mDebugMessenger;
        VkSurfaceKHR mSurface;
        QueueFamilyData mQueueData;
        VkQueue mGraphicsQueue;
        VkQueue mPresentationQueue;
        VkSwapchainKHR mSwapchain;
		std::vector<VkImageView> vecSwapImageViews;
		std::vector<VkImage> vecSwapImages;
		VkFormat mSwapFormat;
		VkExtent2D mSwapExtent;
		VkRenderPass mRenderPass;
		std::vector<VkFramebuffer> vecFrameBuffers;
		VkCommandPool mCmdPool;
		std::vector<VkSemaphore> vecImgAvSemaphores;
		std::vector<VkSemaphore> vecRenderFinsihSemaphores;
		std::vector<VkFence> vecFlightFences;
		std::vector<VkCommandBuffer> vecCmdBuffers;
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

    private:
    };

#endif
}