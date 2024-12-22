#pragma once
#include "Core.hxx"

namespace Ngine
{
#if defined(TARGET_PLATFORM_WINDOWS) || defined(TARGET_PLATFORM_XBOX)
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