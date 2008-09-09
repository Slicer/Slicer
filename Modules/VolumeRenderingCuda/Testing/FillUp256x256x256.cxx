#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>



template <typename InType>
void ResizeArray(InType* in, InType* out, unsigned int resolution[3])
{
    int i = 0;
    for (unsigned int x = 0; x < resolution[0]; x++)
        for (unsigned int y = 0; y < resolution[1]; y++)
            for (unsigned int z = 0; z < resolution[2]; z++)
            {
                out[x + 256*y + 256*256*z] = in[x + y * resolution[0] + z * resolution[0] * resolution[1]];
                /*if ( in[x + y * resolution[0] + z * resolution[0] * resolution[1]] != 0)
                {
                std::cout << x << "x" << y<<"x"<<z<< ": "<< (int)in[x + y * resolution[0] + z * resolution[0] * resolution[1]] << std::endl;
                i = 0;
                }*/
            }
}


template <typename InType>
void ConvertFile(const std::string& inFile, 
                 const std::string& outFile, 
                 unsigned int resolution[3])
{
    unsigned long inSize = resolution[0] * resolution[1] * resolution[2];
    InType* inArray = new InType[inSize];
    InType* outArray = new InType[256*256*256];

//    for (unsigned int i = 0; i < 256*256*256; i++)
//        outArray[i] = 0;


    std::cout << "Read File " << inFile << std::endl;
    FILE* InFile = fopen(inFile.c_str(), "r");
    fread(inArray, sizeof(InType), inSize, InFile);
    fclose(InFile);

    ResizeArray(inArray, outArray, resolution);

    std::cout << " Write to File " << outFile << std::endl;
    FILE* OutFile = fopen(outFile.c_str(), "w");
    fwrite(outArray, sizeof(unsigned char), 256*256*256, OutFile);
    fclose(OutFile);

    delete[] inArray;
}

int main(int argc, char** argv)
{

    if (argc < 7)
    {
        std::cout << "Usage: " << argv[0] << " [InFile] [OutFile] [Type] [ResolutionX] [ResolutionY] [ResolutionZ]\n";

        std::cout << "Type: 1:short, 2:float";

        return 0;
    }

    std::string inFileName = argv[1];
    std::string outFileName = argv[2];
    std::string typeStr = argv[3];

    std::stringstream s;

    unsigned int resolution[3];

    s << argv[4];
    s >> resolution[0];
    s.clear();
    s << argv[5];
    s >> resolution[1];
    s.clear();
    s << argv[6];
    s >> resolution[2];
    s.clear();

    std::cout << "InputFileName: " << inFileName << " OutputFileName: " << outFileName <<
        " Format: " << typeStr <<
        " Resolution: " << resolution[0] << "x" << resolution[1] << "x" << resolution[2] << std::endl;

    if (typeStr == "0" || typeStr == "uchar")
        ConvertFile<unsigned char>(inFileName, outFileName, resolution);
    if (typeStr == "1" || typeStr == "short")
        ConvertFile<short>(inFileName, outFileName, resolution);
}

