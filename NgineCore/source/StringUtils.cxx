#include "StringUtils.h"

namespace Ngine
{
	std::string StringUtils::ConvertToString(const std::wstring& w)
	{
		std::string result = std::string(w.begin(), w.end());
		return result;
	}

	std::string StringUtils::SetToLowercase(std::string s)
	{
		std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {return std::tolower(c); });
		return s;
	}

	std::wstring StringUtils::ConvertToWideString(const std::string& s)
	{
		std::wstring result = std::wstring(s.begin(), s.end());
		return result;
	}
}