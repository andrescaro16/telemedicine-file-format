#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <sstream>
#include <cstring>
#include <jpeglib.h>

using namespace std;

struct ImageInfo {
    int width;
    int height;
    string sexo;
    string nombre;
    string apellido;
    string fecha;
};

ImageInfo readBSONHeader(std::ifstream& file) {
    int headerSize = 0;
    file.read(reinterpret_cast<char*>(&headerSize), sizeof(int));

    // Leer el JSON del encabezado
    std::vector<char> buffer(headerSize + 1, '\0');
    file.read(buffer.data(), headerSize);

    // Parsear el JSON del encabezado para obtener la información de la imagen
    int width = 0, height = 0;
    std::string sexo, nombre, apellido, fecha;
    std::istringstream headerStream(buffer.data());
    headerStream.ignore(headerSize, ':');
    /*
    >> es un operador de extracción que lee datos de un flujo de entrada, como std::cin o un std::istringstream.
    La extracción se detiene automáticamente hasta encontrar espacio en blanco, una nueva línea o el final del flujo.
    */
    headerStream >> height; // Leer el valor de la altura
    headerStream.ignore(headerSize, ':'); // Ignorar hasta el segundo valor
    headerStream >> width;
    headerStream.ignore(headerSize, ':');
    headerStream >> sexo;
    headerStream.ignore(headerSize, ':');
    headerStream >> nombre;
    headerStream.ignore(headerSize, ':');
    headerStream >> apellido;
    headerStream.ignore(headerSize, ':');
    headerStream >> fecha;

    cout << "Ancho: " << width << " - Alto: " << height << " - Nombre: " << nombre << " - Apellido: " << apellido << " - Sexo: " << sexo << " - Fecha: " << fecha << endl;
    return {width, height, sexo, nombre, apellido, fecha};
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