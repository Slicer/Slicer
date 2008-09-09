#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>

#include <float.h>
#include <limits.h>

template <typename Type>
 void GetTypeRange(Type& min, Type& max);
 
 
 template <>
  void GetTypeRange<float>(float& min, float& max)
  {
    min = FLT_MIN; max = FLT_MAX;
  }
  template<>
  void GetTypeRange<short>(short& min, short& max)
  {
    min = SHRT_MIN; max = SHRT_MAX;
  }

template <typename InType>
 void ConvertArray(InType* in, unsigned char* out, unsigned long size)
 {
   unsigned long i;
   InType min, max;
   // get inverse values to evaluate right ones later.
   GetTypeRange(max, min);
   
   // get min and max values
   for (i = 0; i < size; i++)
   {
     if (in[i] < min) min = in[i];
     if (in[i] > max) max = in[i];
   }
   if (min > max)
     return;  
   
   // conversion
   for ( i = 0; i < size; i ++)
   {
    out[i] =  (unsigned char)((in[i] - min) * 256 / (max - min));
          //std::cout << "Convert: " << in[i] << " to " << (int)out[i] << std::endl; 
   }
 }


template <typename InType>
  void ConvertFile(const std::string& inFile, 
                         const std::string& outFile, 
               unsigned int resolution[3])
{
  unsigned long size = resolution[0] * resolution[1] * resolution[2];
  InType* inArray = new InType[size];
  unsigned char* outArray = new unsigned char[size];
  
  std::cout << "Read File " << inFile << std::endl;
   FILE* InFile = fopen(inFile.c_str(), "r");
   fread(inArray, sizeof(InType), size, InFile);
   fclose(InFile);
   
   ConvertArray(inArray, outArray, size);
   
   std::cout << " Write to File " << outFile << std::endl;
   FILE* OutFile = fopen(outFile.c_str(), "w");
   fwrite(outArray, sizeof(unsigned char), size, OutFile);
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

  if (typeStr == "1" || typeStr == "short")
    ConvertFile<short>(inFileName, outFileName, resolution);
}

