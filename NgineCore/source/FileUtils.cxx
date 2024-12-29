#include "FileUtils.h"
#include "StringUtils.h"
#include <cstdint>
#include <fstream>

namespace Ngine
{
	std::string FileUtils::CutPathToFileName(std::string path)
	{
		std::filesystem::path p(path);
		p = p.filename(); //Cut provided path to only file name
		std::string result = p.string(); //Convert filename into string
		return result; //Return result
	}

	std::wstring FileUtils::CutPathToFileName(std::wstring path)
	{
		std::filesystem::path p(path);
		p = p.filename(); //Cut provided path to only file name
		std::wstring result = p.wstring(); //Convert filename into wide string
		return result; //Return result
	}

	std::string FileUtils::CutPathToFileExtension(std::string path)
	{
		std::filesystem::path p(path);
		p = p.extension(); //Cut provided path to only file extension
		std::string result = p.string(); //Convert extension into string
		return result; //Return result
	}

	std::wstring FileUtils::CutPathToFileExtension(std::wstring path)
	{
		std::filesystem::path p(path);
		p = p.extension(); //Cut provided path to only file extension
		std::wstring result = p.wstring(); //Convert extension into wide string
		return result; //Return result
	}

	bool FileUtils::FileExists(std::string path)
	{
		std::ifstream f(path);
		return f.good();
	}
	int FileUtils::GetIntegerFromConfig(std::string file, std::string section, std::string key)
	{
		mINI::INIFile iniFile(file);                        //Create ini file interface
		mINI::INIStructure iniStruct;                       //Create structure for stroing data

		if (!iniFile.read(iniStruct))                       //Read data from file
			return 0;                                       //If function failed return 0

		try
		{
			return std::stoi(iniStruct[section][key]);      //Convert from string to int
		}
		catch (const std::invalid_argument& ia) //In case of exception return 0
		{
			return 0;
		}
		catch (const std::out_of_range& oor)
		{
			return 0;
		}
	}
	float FileUtils::GetFloatFromConfig(std::string file, std::string section, std::string key)
	{
		mINI::INIFile iniFile(file); //Create ini file interface
		mINI::INIStructure iniStruct; //Create structure for stroing data

		if (!iniFile.read(iniStruct)) //Read data from file
			return 0; //If function failed return 0

		try
		{
			return std::stof(iniStruct[section][key]); //Convert from string to float
		}
		catch (const std::invalid_argument& ia) //In case of exception return 0
		{
			return 0;
		}
		catch (const std::out_of_range& oor)
		{
			return 0;
		}
	}
	std::string FileUtils::GetStringFromConfig(std::string file, std::string section, std::string key)
	{
		mINI::INIFile iniFile(file); //Create ini file interface
		mINI::INIStructure iniStruct; //Create structure for stroing data

		if (!iniFile.read(iniStruct)) //Read data from file
			return ""; //If function failed return empty string

		return iniStruct[section][key];
	}

	bool FileUtils::GetBoolFromConfig(std::string file, std::string section, std::string key)
	{
		std::string cfgString = GetStringFromConfig(file, section, key); //Obtain string from config that will be compared
		cfgString = StringUtils::SetToLowercase(cfgString); //Set text to lowercase
		if (strcmp(cfgString.c_str(), "true") == 0) //Compare string with expected results
			return true;
		else
			return false;

	}

	std::vector<std::string> FileUtils::GetFileNamesFromDirectory(std::string path)
	{
		if (!std::filesystem::is_directory(path) || !std::filesystem::exists(path))
		{
			LOG_F(ERROR, "Provided path is not a valid dierctory!");
			return std::vector<std::string>();
		}

		std::vector<std::string> result;
		for (const auto& dir_entry : std::filesystem::directory_iterator{ path })
		{
			if (dir_entry.is_directory())
			{
				LOG_F(INFO, "Subdirectory found: %s", dir_entry.path().c_str());
				continue;
			}

			std::filesystem::path p = dir_entry.path();
			std::string s = p.string();

			LOG_F(INFO, "File found: %s", s.c_str());
			result.push_back(s);
		}

		return result;
	}

	BinaryConfig FileUtils::LoadBinaryConfig(std::string file)
	{
		std::ifstream cFile(file.c_str(), std::ifstream::binary | std::ifstream::in);

		if(!cFile.is_open())
		{
			LOG_F(ERROR, "Cannot open file: %s", file.c_str());
			return BinaryConfig();
		}

		uint32_t numBytesToMove = 0;
		BinaryConfig result;

		//Read and save first byte( Window fullscreen)
		cFile.read(reinterpret_cast<char*>(&result.WindowFullscreen), sizeof(bool));
		//Move by 1 byte 
		numBytesToMove += sizeof(bool);
		cFile.seekg(numBytesToMove);

		//Read and save next 4 bytes (Window width)
		cFile.read(reinterpret_cast<char*>(&result.WindowWidth), sizeof(uint32_t));
		//Move by 4 bytes
		numBytesToMove += sizeof(uint32_t);
		cFile.seekg(numBytesToMove);		

		//Read and save next 4 bytes (Window height)
		cFile.read(reinterpret_cast<char*>(&result.WindowHeight), sizeof(uint32_t));
		//Move by 4 bytes
		numBytesToMove += sizeof(uint32_t);
		cFile.seekg(numBytesToMove);

		//Read and save next byte (Auto pick device)
		cFile.read(reinterpret_cast<char*>(&result.AutoPickDevice), sizeof(bool));
		//Move by 1 byte 
		numBytesToMove += sizeof(bool);
		cFile.seekg(numBytesToMove);

		//Read and save next 2 bytes (Manual device index)
		cFile.read(reinterpret_cast<char*>(&result.ManualDeviceIndex), sizeof(uint16_t));
		//Move by 2 bytes
		numBytesToMove += sizeof(uint16_t);
		cFile.seekg(numBytesToMove);

		//Read and save next byte (Enable gfx debug mode)
		cFile.read(reinterpret_cast<char*>(&result.EnableGfxDebugMode), sizeof(bool));
		//Move by 1 byte 
		numBytesToMove += sizeof(bool);
		cFile.seekg(numBytesToMove);

		//Read and save next byte (Window resize)
		cFile.read(reinterpret_cast<char*>(&result.WindowResize), sizeof(bool));

		cFile.close();

		return result;
	}
}