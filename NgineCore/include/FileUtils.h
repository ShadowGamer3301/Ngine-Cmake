#pragma once
#include "Core.hxx"

namespace Ngine
{
#if defined(TARGET_PLATFORM_WINDOWS) || defined(TARGET_PLATFORM_XBOX)
	class NGAPI FileUtils;
#endif

	struct BinaryConfig
	{
		bool WindowFullscreen = false;
		uint32_t WindowWidth = 0;
		uint32_t WindowHeight = 0;
		bool AutoPickDevice = false;
		uint16_t ManualDeviceIndex = 0;
		bool EnableGfxDebugMode = false;
		bool WindowResize = false;
	};

	class FileUtils
	{
	public:
		static std::string CutPathToFileName(std::string path);
		static std::wstring CutPathToFileName(std::wstring path);
		static std::string CutPathToFileExtension(std::string path);
		static std::wstring CutPathToFileExtension(std::wstring path);
		static bool FileExists(std::string path);
		static int GetIntegerFromConfig(std::string file, std::string section, std::string key);
		static float GetFloatFromConfig(std::string file, std::string section, std::string key);
		static std::string GetStringFromConfig(std::string file, std::string section, std::string key);
		static bool GetBoolFromConfig(std::string file, std::string section, std::string key);
		static std::vector<std::string> GetFileNamesFromDirectory(std::string path);
		static BinaryConfig LoadBinaryConfig(std::string file);
	};
}