#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <jpeglib.h>

struct ImageInfo {
    int width;
    int height;
};

void writeBSONHeader(std::ofstream& file, int width, int height) {
    int headerSize = sizeof(int) * 2; // Tamaño de la cabecera BSON (2 enteros)
    file.write(reinterpret_cast<const char*>(&headerSize), sizeof(int));
    file.write("{\"height\":", 10);
    file.write(reinterpret_cast<const char*>(&height), sizeof(int));
    file.write(",\"width\":", 9);
    file.write(reinterpret_cast<const char*>(&width), sizeof(int));
}

void createMexFile(const std::string& jpgFileName, const std::string& mexFileName) {
    FILE* infile = fopen(jpgFileName.c_str(), "rb");
    if (!infile) {
        std::cerr << "Error: No se pudo abrir la imagen " << jpgFileName << std::endl;
        return;
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    ImageInfo imageInfo;
    imageInfo.width = cinfo.output_width;
    imageInfo.height = cinfo.output_height;

    std::vector<uint8_t> imageData(imageInfo.width * imageInfo.height * cinfo.num_components);
    JSAMPARRAY buffer = new JSAMPROW[cinfo.output_height];
    for (int i = 0; i < cinfo.output_height; ++i) {
        buffer[i] = new JSAMPLE[cinfo.output_width * cinfo.num_components];
    }

    int rowStride = cinfo.output_width * cinfo.num_components;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        std::memcpy(&imageData[(cinfo.output_scanline - 1) * rowStride], buffer[0], rowStride);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(infile);

    std::ofstream outputFile(mexFileName, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << mexFileName << " para escritura." << std::endl;
        return;
    }

    writeBSONHeader(outputFile, imageInfo.width, imageInfo.height);
    outputFile.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
    outputFile.close();

    std::cout << "Archivo " << mexFileName << " creado exitosamente." << std::endl;
}

int main() {
    std::string jpgFileName = "imagen.jpg";
    std::string mexFileName = "imagen.mex";
    createMexFile(jpgFileName, mexFileName);
    return 0;
}