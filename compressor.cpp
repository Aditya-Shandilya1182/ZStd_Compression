#include <iostream>
#include <fstream>
#include <string>
#include <zstd.h>
#include <vector>
#define ZSTD_LEVEL 15

bool compressFile(const std::string& inputFileName, const std::string& outputFileName){
    
    std::ifstream inputFile(inputFileName, std::ios::binary);
    if(!inputFile){
        std::cerr << "Error: input file not found: " << inputFileName << std::endl;
        return false;
    }

    std::ofstream outputFile(outputFileName, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: could not create output file " << outputFileName << std::endl;
        return false;
    }

    bool success = zstdCompressor(inputFile, outputFile);

    inputFile.close();
    outputFile.close();

    return success;

}

bool compressString(const std::string& inputString, const std::string& outputFileName){

    std::ofstream outputFile(outputFileName, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: could not create output file " << outputFileName << std::endl;
        return false;
    }

    bool success = zstdStringCompressor(inputString, outputFile);

    outputFile.close();

    return success;
}

bool zstdCompressor(std::ifstream& ifs, std::ofstream& ofs){
    
    size_t bufferSize = ZSTD_CStreamInSize() + ZSTD_CStreamOutSize();
    std::vector<char> buffer(bufferSize);

    ZSTD_CCtx* zCompressorContext = ZSTD_createCCtx();
    if(!zCompressorContext){
        std::cerr << "Error: could not create zstd compression context" << std::endl;
        return false;
    }

    while(ifs){
        ifs.read(buffer.data(), buffer.size());
        size_t bytesRead = ifs.gcount();
        if(bytesRead == 0){
            break;
        }

        ZSTD_inBuffer input = { buffer.data(), bytesRead, 0 };
        while(input.pos < input.size){
            ZSTD_outBuffer output = { buffer.data(), buffer.size(), 0 };
            size_t bytesWritten = ZSTD_compressStream(zCompressorContext, &output, &input);
            if(ZSTD_isError(bytesWritten)){
                std::cerr << "Error: ZSTD compression failed" << std::endl;
                ZSTD_freeCCtx(zCompressorContext);
                return false;
            }
            ofs.write(buffer.data(), output.pos);
        }
    }
    ZSTD_freeCCtx(zCompressorContext);
    return true;

}

bool zstdStringCompressor(const std::string& inputString, std::ofstream& ofs){
    
    size_t maxCompressedSize = ZSTD_compressBound(inputString.size());
    std::string compressed(maxCompressedSize, '\0');

    size_t compressedSize = ZSTD_compress(&compressed[0], compressed.size(), inputString.data(), inputString.size(), ZSTD_LEVEL);
    if (ZSTD_isError(compressedSize)) {
        std::cerr << "Error: ZSTD compression failed" << std::endl;
        return false;
    }

    ofs.write(compressed.data(), compressedSize);
    return true;

}

double sizeByteMetric(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error: failed to open file for size metric" << std::endl;
        return -1.0;
    }
    return static_cast<double>(file.tellg());
}

int main(){

    std::string inputFilename = "input.txt";
    std::string outputFilename = "output.zst";

    bool success = compressFile(inputFilename, outputFilename);
    if (success) {
        std::cout << "Compression successful!" << std::endl;
    } else {
        std::cerr << "Compression failed!" << std::endl;
    }
    
    return 0;
}