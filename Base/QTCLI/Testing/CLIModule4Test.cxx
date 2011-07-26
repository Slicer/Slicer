/*=========================================================================

  Program:   Slicer

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "CLIModule4TestCLP.h"

// STD includes
#include <fstream>

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//

namespace
{

bool outputResult(int result, const std::string& outputFile)
  {
  std::ofstream myfile;
  myfile.open(outputFile.c_str());

  if (!myfile.is_open())
    {
      std::cerr << "Failed to open file:" << outputFile << std::endl;
      return false;
    }

  myfile << result << "\n";

  myfile.close();
  return true;
  }

} // end of anonymous namespace


int main(int argc, char * argv[])
{

  PARSE_ARGS;

  int result = 0;

  if (OperationType == std::string("Addition"))
    {
    result = InputValue1 + InputValue2;
    }
  else if (OperationType == std::string("Multiplication"))
    {
    result = InputValue1 * InputValue2;
    }
  else
    {
    std::cerr << "Unknown OperationType:" << OperationType << std::endl;
    return EXIT_FAILURE;
    }

  if (!outputResult(result, OutputFile))
    {
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
