#pragma once
#include "Core.hxx"
#include <source_location>
#include <vulkan/vulkan_core.h>

namespace Ngine
{
#if defined(WIN32) || defined(_WIN32)
	class NGAPI Excpetion;
	class NGAPI DirectXException;
#endif

	class Exception : public std::exception
	{
	public:
		Exception(std::source_location loc = std::source_location::current());
		const char* what() const noexcept;

	protected:
		unsigned int line;
		std::string file, func;
		mutable std::string wBuffer;
	};

#if defined(WIN32) || defined(_WIN32)
	class DirectXException : public Exception
	{
	public:
		DirectXException(HRESULT hr, std::source_location loc = std::source_location::current());
		const char* what() const noexcept;

	private:
		HRESULT code;
	};
#elif defined(__linux__) || defined (linux)

	class VulkanException : public Exception
	{
	public:
		VulkanException(VkResult res, std::source_location loc = std::source_location::current());
		const char* what() const noexcept;

	private:
		VkResult code;
	};
#endif
}