/*=========================================================================

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include "CLIROITestCLP.h"
#include "vtkPluginFilterWatcher.h"


// VTK includes
#include <vtkMath.h>

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

} // end of anonymous namespace

//-----------------------------------------------------------------------------
int main(int argc, char * argv[])
{
  PARSE_ARGS;

  for (size_t i=0; i<ROI_One.size(); i++)
    {
    std::cout << "ROI_In[" << i << "]" << ROI_One[i] << ",";
    }

  for (size_t j=0; j<ROI_List.size(); j++)
    {
    for (size_t i=0; i<ROI_List[j].size(); i++)
      {
      std::cout << "ROI_List["<< j << "][" << i << "]" << ROI_List[j][i] << ",";
      }
    }

  return EXIT_SUCCESS;
}
