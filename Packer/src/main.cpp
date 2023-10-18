#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string>

// PACKER v1.0
// -----------------------------------
// STRUCTURE:
// Header length
// Headers
// - Name of file (e.g. background.jpg)
// - Offset
// - Length
// Files Packed

constexpr size_t MAX_FILENAME_LENGTH = 64;
constexpr size_t BUFFER_SIZE = 4096;

// Util function to check the absolute path we are trying to reach
// Useful where relative path is confusing.
std::string PrintAbsolutePath(const std::string& relativePath) {
	std::filesystem::path path(relativePath);
	if (path.is_relative()) {
		std::filesystem::path absolutePath = std::filesystem::current_path() / path;
		return absolutePath.string();
	}
	else {
		return path.string();
	}
}

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

	// Precalc header size based on struct size (fixed) and amount of files to be packed
	uint16_t headersSize = sizeof(AssetHeader) * (argc - 2);
	std::vector<AssetHeader> headers;
	// Since we know the number of files, we can reserve memory for vector
	headers.reserve(argc - 2);

	// Write headers size
	output.write(reinterpret_cast<const char*>(&headersSize), sizeof(uint16_t));

	// Move cursor to end of headers + headerlength bytes
	output.seekp(headersSize + sizeof(headersSize), std::ios::beg);

	// Set the current offset to the beginning of the files data
	uint32_t offset = headersSize;

	for (int i = 2; i < argc; ++i) {
		std::filesystem::path filePath(argv[i]);
		std::string fileName = filePath.filename().string();

		// Check if file name is not too long
		if (fileName.size() > MAX_FILENAME_LENGTH) {
			std::cerr << "Filename is too long: " << fileName << std::endl;
			return 1;
		}

		std::ifstream input(argv[i], std::ios::binary);
		if (!input.is_open()) {
			std::cerr << "Couldn't open file: " << PrintAbsolutePath(argv[i]) << std::endl;
			return 1;
		}

		// Create buffer for copying file data
		char buffer[BUFFER_SIZE];
		uint32_t fileLength = 0;

		while (!input.eof()) {
			input.read(buffer, BUFFER_SIZE);
			std::streamsize bytesRead = input.gcount();
			output.write(buffer, bytesRead);
			fileLength += bytesRead;
		}

		input.close();

		// Create header for file
		AssetHeader header;
		strncpy(header.name, fileName.c_str(), MAX_FILENAME_LENGTH - 1);
		header.name[fileName.size()] = '\0';
		header.offset = offset;
		header.length = fileLength;

		// Add header to headers vector
		headers.push_back(header);

		offset += fileLength;
	}

	// Return the cursor to the beginning (plus the header length bytes (2)) of the file to overwrite the placeholder headers
	output.seekp(sizeof(uint16_t), std::ios::beg);
	for (const auto& header : headers) {
		// Write the headers
		output.write(header.name, MAX_FILENAME_LENGTH);
		output.write(reinterpret_cast<const char*>(&header.offset), sizeof(header.offset));
		output.write(reinterpret_cast<const char*>(&header.length), sizeof(header.length));
	}

	output.close();

	std::cout << "Success! The following files have been successfully packed into " << argv[1] << ":" << std::endl;
	for (int i = 2; i < argc; ++i) {
		std::filesystem::path filePath(argv[i]);
		std::cout << " - " << PrintAbsolutePath(argv[i]) << "\n" << std::endl;
	}

	return 0;
}