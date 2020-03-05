/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

/*=========================================================================
  Program:   Slicer
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
