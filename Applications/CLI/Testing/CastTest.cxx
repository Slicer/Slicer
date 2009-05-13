#include <iostream>
#include "itkTestMain.h" 
void RegisterTests()
{
  REGISTER_TEST(CastTest);
}
#undef main
#define main CastTest
#include "Cast.cxx"
