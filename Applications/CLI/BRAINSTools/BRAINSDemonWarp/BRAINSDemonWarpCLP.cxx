#include <iostream>
#include "BRAINSDemonWarpTemplates.h"

// main function built in BRAINSDemonWarpPrimary.cxx so that testing only builds
// templates once.

int main(int argc, char *argv[])
{
  std::cout << "DEPRECATED:" << argv[0] << std::endl;
  std::cout << "DEPRECATED:" << argv[0] << std::endl;
  std::cout << "DEPRECATED:" << argv[0] << std::endl;
  std::cout << "DEPRECATED:  You should use BRAINSDemonWarp instead." << std::endl;
  std::cout << "DEPRECATED:  BRAINSDemonWarp has the same command line." << std::endl;
  std::cout << "DEPRECATED:" << argv[0] << std::endl;
  std::cout << "DEPRECATED:" << argv[0] << std::endl;
  std::cout << "DEPRECATED:" << argv[0] << std::endl;

  return BRAINSDemonWarpPrimary(argc, argv);
}
