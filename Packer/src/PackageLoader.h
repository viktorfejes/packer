#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

class PackageLoader {
public:
	PackageLoader(const std::string& packageFile);

	static std::vector<uint8_t> loadSingleAsset(const std::string& packageFile, const std::string& asset);
	void loadAsset(const std::string& assetName, std::vector<uint8_t>& output) const;

private:
	struct PackageHeader {
		uint32_t offset;
		uint32_t length;
	};

	std::string m_PackageFilePath;
	uint16_t m_HeaderLength;
	std::unordered_map<std::string, PackageHeader> m_Headers;
};