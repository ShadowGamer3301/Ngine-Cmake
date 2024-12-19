#pragma once
#include "Core.hxx"

namespace Ngine
{
#if defined(WIN32) || defined(_WIN32)
	class NGAPI StringUtils;
#endif

	class StringUtils
	{
	public:
		static std::wstring ConvertToWideString(const std::string& s);
		static std::string ConvertToString(const std::wstring& w);
		static std::string SetToLowercase(std::string s);
	};
}