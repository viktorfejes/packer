#include "PackageLoader.h"

PackageLoader::PackageLoader(const std::string& packageFile)
{
	std::ifstream pkg(packageFile, std::ios::binary);
	if (!pkg.is_open()) {
		throw std::runtime_error("Couldn't open package file: " + packageFile);
	}

	// Read in the header length bytes (first 2)
	pkg.read(reinterpret_cast<char*>(&m_HeaderLength), 2);
	if (m_HeaderLength % sizeof(PackageHeader) != 0) {
		throw std::runtime_error("Invalid header length in packaged file: " + packageFile);
	}

	// Clear out to make sure.
	m_Headers.clear();

	int lengthCheck = 0;

	while (pkg.tellg() < m_HeaderLength) {
		PackageHeader header;
		char nameBuffer[64];
		pkg.read(nameBuffer, 64);
		pkg.read(reinterpret_cast<char*>(&header.offset), sizeof(uint32_t));
		pkg.read(reinterpret_cast<char*>(&header.length), sizeof(uint32_t));

		if (pkg.fail()) {
			throw std::runtime_error("Failed to read header from package file: " + packageFile);
		}

		// This way we can check the size of our file
		lengthCheck += header.length;

		// Create string out of buffer and use it for the unordered_map
		std::string assetName(nameBuffer);
		m_Headers[assetName] = header;
	}

	// Sanity check to make sure the file is not corrupted
	pkg.seekg(0, std::ios::end);
	if (pkg.tellg() < lengthCheck + m_HeaderLength) {
		throw std::runtime_error("Package file seems to be corrupted: " + packageFile);
	}

	// Let's save as a member variable, since everything seems to be ok
	m_PackageFilePath = packageFile;

	// Close file just in case
	pkg.close();
}

std::vector<uint8_t> PackageLoader::loadSingleAsset(const std::string& packageFile, const std::string& asset)
{
	return std::vector<uint8_t>();
}

void PackageLoader::loadAsset(const std::string& assetName, std::vector<uint8_t>& output) const {
	std::ifstream pkg(m_PackageFilePath, std::ios::binary);
	if (!pkg.is_open()) {
		throw std::runtime_error("Couldn't open package file: " + m_PackageFilePath);
	}

	// See if we can find the asset in the headers
	auto findAsset = m_Headers.find(assetName);
	if (findAsset == m_Headers.end()) {
		throw std::runtime_error("Asset \"" + assetName + "\" not found in currently loaded package.");
	}

	const PackageHeader& header = findAsset->second;

	// Let's jump to the right offset,
	// make sure the output vector is the right size,
	// and load the file based on offset and length into the output vector
	output.resize(header.length);
	pkg.seekg(header.offset, std::ios::beg);
	pkg.read(reinterpret_cast<char*>(output.data()), header.length);

	if (pkg.fail() || pkg.gcount() != header.length) {
		throw std::runtime_error("Failed to read header from package file: " + m_PackageFilePath);
	}

	pkg.close();
}
