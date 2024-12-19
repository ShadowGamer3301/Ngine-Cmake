#pragma once
#include "Core.hxx"

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
#endif
}