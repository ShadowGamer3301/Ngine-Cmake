#pragma once
#include "Core.hxx"

namespace Ngine
{
#if defined(TARGET_PLATFORM_WINDOWS)
	class NGAPI Excpetion;
	class NGAPI DirectXException;
#elif defined(TARGET_PLATFORM_XBOX)
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

#if defined(TARGET_PLATFORM_WINDOWS) || defined(TARGET_PLATFORM_XBOX)
	class DirectXException : public Exception
	{
	public:
		DirectXException(HRESULT hr, std::source_location loc = std::source_location::current());
		const char* what() const noexcept;

	private:
		HRESULT code;
	};
#elif defined(TARGET_PLATFORM_LINUX)

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