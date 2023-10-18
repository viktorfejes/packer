#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string>

// PACKER v1.0
// -----------------------------------
// STRUCTURE:
// Header length
// Header
// - Name of file (e.g. background.jpg)
// - Offset
// - Length
// Files Packed

constexpr size_t MAX_FILENAME_LENGTH = 64;

struct AssetHeader {
	char name[MAX_FILENAME_LENGTH];
	uint32_t offset, length;
};

int main(int argc, char** argv) {
	if (argc < 3) {
		std::cerr << "Usage: Packer <output pkg file> <asset1> <asset2> ..." << std::endl;
		return 1;
	}

	std::ofstream output(argv[1], std::ios::binary | std::ios::out);
	if (!output.is_open()) {
		std::cerr << "Couldn't create output file!" << std::endl;
		return 1;
	}

	int headerSize = 0;
	std::vector<AssetHeader> headers;
	std::vector<uint8_t> files;

	for (int i = 2; i < argc; ++i) {
		std::filesystem::path filePath(argv[i]);
		std::string fileName = filePath.filename().string();

		std::ifstream input(argv[i], std::ios::binary);
		if (!input.is_open()) {
			std::cerr << "Couldn't open file: " << argv[i] << std::endl;
			return 1;
		}

		// Load binary into vector of uint8_t
		std::vector<uint8_t> inputBin(std::istreambuf_iterator<char>(input), {});
		input.close();

		// Create header for file
		AssetHeader header;
		header.name = filePath.filename().string() + "\0"; // with null terminator
		header.offset = (headers.size() + header.name.size() + (sizeof(uint32_t) * 2)) + files.size();
		header.length = inputBin.size();

		// Add header to headers vector
		headers.push_back(header);
		// Insert into the binaries vector for files
		files.insert(files.end(), inputBin.begin(), inputBin.end());
	}

	for (const auto& header : headers) {
		// Write the headers
		output.write(header.name.data(), header.name.size());
		output.write(reinterpret_cast<const char*>(&header.offset), sizeof(header.offset));
		output.write(reinterpret_cast<const char*>(&header.length), sizeof(header.length));
	}

	output.write(reinterpret_cast<const char*>(files.data()), files.size());

	return 0;
}