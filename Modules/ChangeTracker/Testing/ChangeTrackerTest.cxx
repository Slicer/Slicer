#include <iostream>
#include "itkTestMain.h" 
void RegisterTests()
{
    REGISTER_TEST(ChangeTrackerTest);
}
#undef main
#define main ChangeTrackerTest
#include "ChangeTrackerCommandLine.cxx"
