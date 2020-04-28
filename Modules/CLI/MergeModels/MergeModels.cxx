/*=========================================================================

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "MergeModelsCLP.h"

// VTK includes
#include <vtkAppendPolyData.h>

// MRML includes
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"


int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  vtkNew<vtkMRMLModelStorageNode> model1StorageNode;
  model1StorageNode->SetFileName(Model1.c_str());
  vtkNew<vtkMRMLModelNode> model1Node;
  if (!model1StorageNode->ReadData(model1Node))
    {
    std::cerr << "Failed to read input model 1 file " << Model1 << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLModelStorageNode> model2StorageNode;
  model2StorageNode->SetFileName(Model2.c_str());
  vtkNew<vtkMRMLModelNode> model2Node;
  if (!model2StorageNode->ReadData(model2Node))
    {
    std::cerr << "Failed to read input model 2 file " << Model2 << std::endl;
    return EXIT_FAILURE;
    }

  // add them together
  vtkNew<vtkAppendPolyData> add;
  add->AddInputConnection(model1Node->GetPolyDataConnection());
  add->AddInputConnection(model2Node->GetPolyDataConnection());
  add->Update();

  vtkNew<vtkMRMLModelNode> outputModelNode;
  outputModelNode->SetAndObservePolyData(add->GetOutput());
  vtkNew<vtkMRMLModelStorageNode> outputModelStorageNode;
  outputModelStorageNode->SetFileName(ModelOutput.c_str());
  if (!outputModelStorageNode->WriteData(outputModelNode))
    {
    std::cerr << "Failed to write output model file " << ModelOutput << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
