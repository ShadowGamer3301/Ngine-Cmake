#include "Exception.h"

namespace Ngine
{
	Exception::Exception(std::source_location loc)
		: file(loc.file_name()), func(loc.function_name()), line(loc.line())
	{}

	const char* Exception::what() const noexcept
	{
		std::ostringstream oss;
		oss << "Exception caught!\n"
			<< "File: " << file << "\n"
			<< "Func: " << func << "\n"
			<< "Line: " << line << "\n";

		wBuffer = oss.str();
		return wBuffer.c_str();
	}

	DirectXException::DirectXException(HRESULT hr, std::source_location loc)
		: code(hr), Exception(loc)
	{}

	const char* DirectXException::what() const noexcept
	{
		std::ostringstream oss;
		oss << "Exception caught!\n"
			<< "Code: " << std::hex << code << std::dec << "\n"
			<< "File: " << file << "\n"
			<< "Func: " << func << "\n"
			<< "Line: " << line << "\n";

		wBuffer = oss.str();
		return wBuffer.c_str();
	}
}