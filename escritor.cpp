#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdint>
#include <cstring>
#include <jpeglib.h>
#include <limits>

using namespace std;


struct ImageInfo {
    int width;
    int height;
};

void writeBSONHeader(std::ofstream& file, int width, int height, string sexo, string nombre, string apellido, string fecha) {
    // Escribir el JSON del encabezado en un stringstream
    std::ostringstream headerStream;
    headerStream << "{\"height\":" << height << " ,\"width\":" << width << " ,\"sexo\":\"" << sexo << "\" ,\"nombre\":\"" << nombre << "\" ,\"apellido\":\"" << apellido << "\" ,\"fecha\":\"" << fecha << "\" }";

    // Obtener el tamaño del encabezado
    int headerSize = headerStream.str().size();

    // Escribir el tamaño del encabezado en el archivo
    file.write(reinterpret_cast<const char*>(&headerSize), sizeof(int));

    // Escribir el JSON del encabezado en el archivo
    file.write(headerStream.str().c_str(), headerSize);
}

struct PatientInfo {
    std::string patientName;
    std::string patientLastName;
    int age;
    std::string sex;
    std::string date;
};

PatientInfo getPatientInfoFromCommandLine() {
    PatientInfo info;

    std::cout << "Por favor, introduce el nombre del paciente: ";
    std::getline(std::cin, info.patientName);

    std::cout << "Por favor, introduce el apellido del paciente: ";
    std::getline(std::cin, info.patientLastName);

    std::cout << "Por favor, introduce la edad del paciente: ";
    std::cin >> info.age;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignorar el resto de la línea

    std::cout << "Por favor, introduce el sexo del paciente: ";
    std::getline(std::cin, info.sex);

    std::cout << "Por favor, introduce la fecha: ";
    std::getline(std::cin, info.date);

    return info;
}

void createMexFile(const string& jpgFileName, const string& mexFileName, const PatientInfo& patientInfo) {
    FILE* infile = fopen(jpgFileName.c_str(), "rb");
    if (!infile) {
        cerr << "Error: No se pudo abrir la imagen " << jpgFileName << endl;
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

    vector<uint8_t> imageData(imageInfo.width * imageInfo.height * cinfo.num_components);
    JSAMPARRAY buffer = new JSAMPROW[cinfo.output_height];
    for (int i = 0; i < cinfo.output_height; ++i) {
        buffer[i] = new JSAMPLE[cinfo.output_width * cinfo.num_components];
    }

    int rowStride = cinfo.output_width * cinfo.num_components;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(&imageData[(cinfo.output_scanline - 1) * rowStride], buffer[0], rowStride);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(infile);

    ofstream outputFile(mexFileName, ios::binary);
    if (!outputFile.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << mexFileName << " para escritura." << endl;
        return;
    }

    writeBSONHeader(outputFile, imageInfo.width, imageInfo.height, patientInfo.sex, patientInfo.patientName, patientInfo.patientLastName, patientInfo.date);
    outputFile.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
    outputFile.close();

    cout << "Archivo " << mexFileName << " creado exitosamente." << endl;
}

int main() {
    PatientInfo patientInfo = getPatientInfoFromCommandLine();
    string jpgFileName = "imagen.jpg";
    string mexFileName = "imagen.mex";
    createMexFile(jpgFileName, mexFileName, patientInfo);
    return 0;
}