#include "FreesurferSurfaceSectionExtractionCLP.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiscreteGaussianImageFilter.h"

int main(int argc, char * argv [])
{
 PARSE_ARGS;
 std::cout << "Hello World!" << std::endl;
 return EXIT_SUCCESS;
}
