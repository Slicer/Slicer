#include <iostream>
#include "itkTestMain.h" 
void RegisterTests()
{
    REGISTER_TEST(MRIBiasFieldCorrectionTest);
}
#undef main
#define main MRIBiasFieldCorrectionTest
#include "MRIBiasFieldCorrectionCommandLine.cxx"
