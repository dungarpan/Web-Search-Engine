#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

struct IndexEntry {
    uint32_t key;
    std::vector<uint32_t> docids;
    std::vector<uint32_t> occurrences;
};

// Encode an integer using Variable Byte Encoding
void encodeVByte(std::vector<uint8_t>& result, uint32_t value) {
    do {
        uint8_t byte = value & 0x7F;  // Lower 7 bits
        value >>= 7;
        if (value > 0) {
            byte |= 0x80;  // Set the 8th bit to indicate more bytes
        }
        result.push_back(byte);
    } while (value > 0);
}

// Decode an integer using Variable Byte Encoding
uint32_t decodeVByte(const uint8_t* data, size_t& offset) {
    uint32_t value = 0;
    int shift = 0;
    while (true) {
        uint8_t byte = data[offset++];
        value |= (byte & 0x7F) << shift;
        shift += 7;
        if ((byte & 0x80) == 0) {
            break;  // Last byte
        }
    }
    return value;
}

int main() {
    const int numFiles = 1;

    for (int fileNumber = 0; fileNumber < numFiles; ++fileNumber) {
        std::string inputFileName = "final_ii_" + std::to_string(fileNumber) + ".txt";
        std::string binaryFileName = "index_" + std::to_string(fileNumber) + ".bin";

        std::ifstream inputFile(inputFileName);
        if (!inputFile.is_open()) {
            std::cerr << "Error opening input file: " << inputFileName << std::endl;
            continue;
        }

        std::vector<IndexEntry> index;

        // Read and parse the input file
        std::string line;
        while (std::getline(inputFile, line)) {
            std::istringstream iss(line);
            uint32_t key;
            char colon;
            iss >> key >> colon;

            IndexEntry entry;
            entry.key = key;
            uint32_t docid, occurrence;
            while (iss) {
                iss >> docid;
                iss.ignore(); // Ignore the comma
                iss >> occurrence;
                iss.ignore(); // Ignore the comma
                entry.docids.push_back(docid);
                entry.occurrences.push_back(occurrence);
            }

            index.push_back(entry);
        }

        // Serialize and write the data to a binary file with VByte encoding
        std::ofstream binaryFile(binaryFileName, std::ios::binary);
        if (!binaryFile.is_open()) {
            std::cerr << "Error opening binary file for writing: " << binaryFileName << std::endl;
            continue;
        }

        for (const IndexEntry& entry : index) {
            binaryFile.write(reinterpret_cast<const char*>(&entry.key), sizeof(entry.key));

            uint32_t numValues = static_cast<uint32_t>(entry.docids.size());
            binaryFile.write(reinterpret_cast<const char*>(&numValues), sizeof(numValues));

            for (size_t i = 0; i < numValues; ++i) {
                std::vector<uint8_t> encodedDocid;
                encodeVByte(encodedDocid, entry.docids[i]);
                std::vector<uint8_t> encodedOccurrence;
                encodeVByte(encodedOccurrence, entry.occurrences[i]);

                // Write the VByte encoded docid and occurrence
                binaryFile.write(reinterpret_cast<const char*>(encodedDocid.data()), encodedDocid.size());
                binaryFile.write(reinterpret_cast<const char*>(encodedOccurrence.data()), encodedOccurrence.size());
            }
        }

        binaryFile.close();
        inputFile.close();

        std::cout << "Created binary file with VByte encoding: " << binaryFileName << std::endl;
    }

    return 0;
}
