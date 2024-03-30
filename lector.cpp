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

ImageInfo readBSONHeader(std::ifstream& file) {
    int headerSize;
    file.read(reinterpret_cast<char*>(&headerSize), sizeof(int));

    // Suponemos que la cabecera siempre es válida y contiene solo altura y ancho
    ImageInfo imageInfo;
    file.seekg(headerSize, std::ios::beg);
    file.read(reinterpret_cast<char*>(&imageInfo.height), sizeof(int));
    file.read(reinterpret_cast<char*>(&imageInfo.width), sizeof(int));

    return imageInfo;
}

void readMexFile(const std::string& mexFileName, const std::string& jpgOutputFileName) {
    std::ifstream inputFile(mexFileName, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << mexFileName << " para lectura." << std::endl;
        return;
    }

    ImageInfo imageInfo = readBSONHeader(inputFile);

    std::vector<uint8_t> imageData(imageInfo.width * imageInfo.height * 3); // Suponemos RGB

    inputFile.read(reinterpret_cast<char*>(imageData.data()), imageData.size());
    inputFile.close();

    FILE* outfile = fopen(jpgOutputFileName.c_str(), "wb");
    if (!outfile) {
        std::cerr << "Error: No se pudo crear la imagen " << jpgOutputFileName << std::endl;
        return;
    }

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = imageInfo.width;
    cinfo.image_height = imageInfo.height;
    cinfo.input_components = 3; // RGB
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer[1];
    int row_stride = cinfo.image_width * 3;

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &imageData[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fclose(outfile);

    std::cout << "Imagen reconstruida guardada como " << jpgOutputFileName << std::endl;
}

int main() {
    std::string mexFileName = "imagen.mex";
    std::string jpgOutputFileName = "imagen_reconstruida.jpg";
    readMexFile(mexFileName, jpgOutputFileName);
    return 0;
}