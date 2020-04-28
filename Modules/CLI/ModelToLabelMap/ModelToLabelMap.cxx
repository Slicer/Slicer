/*=========================================================================

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// ModelToLabelMap includes
#include "ModelToLabelMapCLP.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkImageStencil.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkStripper.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>

// MRML includes
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLLabelMapVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkOrientedImageData.h"


int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  // read the poly data
  vtkNew<vtkMRMLModelStorageNode> modelStorageNode;
  vtkNew<vtkMRMLModelNode> modelNode;
  modelStorageNode->SetFileName(surface.c_str());
  if (!modelStorageNode->ReadData(modelNode))
    {
    std::cerr << "Failed to read input model file " << surface << std::endl;
    return EXIT_FAILURE;
    }
  vtkSmartPointer<vtkPolyData> closedSurfacePolyData_RAS = modelNode->GetPolyData();
  if (!closedSurfacePolyData_RAS || closedSurfacePolyData_RAS->GetNumberOfPoints() < 2 || closedSurfacePolyData_RAS->GetNumberOfCells() < 2)
    {
    std::cerr << "Invalid polydata in model file " << surface << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLScalarVolumeNode> referenceVolumeNode;
  vtkNew<vtkMRMLVolumeArchetypeStorageNode> referenceVolumeStorageNode;
  referenceVolumeStorageNode->SetFileName(InputVolume.c_str());
  if (!referenceVolumeStorageNode->ReadData(referenceVolumeNode))
    {
    std::cerr << "Failed to read input volume file " << InputVolume << std::endl;
    return EXIT_FAILURE;
    }
  if (!referenceVolumeNode->GetImageData())
    {
    std::cerr << "Invalid input volume file " << InputVolume << std::endl;
    return EXIT_FAILURE;
    }
  int* referenceVolumeExtent = referenceVolumeNode->GetImageData()->GetExtent();
  if (referenceVolumeExtent[0]>= referenceVolumeExtent[1]
    || referenceVolumeExtent[2] >= referenceVolumeExtent[3]
    || referenceVolumeExtent[4] >= referenceVolumeExtent[5])
    {
    std::cerr << "Empty input volume file " << InputVolume << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkOrientedImageData> binaryLabelmap;
  binaryLabelmap->SetExtent(referenceVolumeExtent);
  binaryLabelmap->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  void* binaryLabelmapVoxelsPointer = binaryLabelmap->GetScalarPointerForExtent(binaryLabelmap->GetExtent());
  if (!binaryLabelmapVoxelsPointer)
    {
    std::cerr << "Failed to allocate memory for output labelmap image" << std::endl;
    return EXIT_FAILURE;
    }
  binaryLabelmap->GetPointData()->GetScalars()->Fill(0); // background voxels are 0

  // Now the output labelmap image data contains the right geometry.
  // We need to apply inverse of geometry matrix to the input poly data so that we can perform
  // the conversion in IJK space, because the filters do not support oriented image data.
  vtkNew<vtkMatrix4x4> ijkToRASMatrix;
  referenceVolumeNode->GetIJKToRASMatrix(ijkToRASMatrix);
  vtkNew<vtkTransform> rasToIJKTransform;
  rasToIJKTransform->SetMatrix(ijkToRASMatrix);
  rasToIJKTransform->Inverse();

  // Leave to identity matrix in binary labelmap volume so that we can perform the stencil operation in IJK space,
  // and now we convert the closed surface to IJK space, too.
  vtkNew<vtkTransformPolyDataFilter> transformPolyDataFilter;
  transformPolyDataFilter->SetInputData(closedSurfacePolyData_RAS);
  transformPolyDataFilter->SetTransform(rasToIJKTransform);

  // Compute polydata normals
  vtkNew<vtkPolyDataNormals> normalFilter;
  normalFilter->SetInputConnection(transformPolyDataFilter->GetOutputPort());
  normalFilter->ConsistencyOn();

  // Make sure that we have a clean triangle polydata
  vtkNew<vtkTriangleFilter> triangle;
  triangle->SetInputConnection(normalFilter->GetOutputPort());

  // Convert to triangle strip
  vtkNew<vtkStripper> stripper;
  stripper->SetInputConnection(triangle->GetOutputPort());

  // Convert polydata to stencil
  vtkNew<vtkPolyDataToImageStencil> polyDataToImageStencil;
  polyDataToImageStencil->SetInputConnection(stripper->GetOutputPort());
  polyDataToImageStencil->SetOutputSpacing(binaryLabelmap->GetSpacing());
  polyDataToImageStencil->SetOutputOrigin(binaryLabelmap->GetOrigin());
  polyDataToImageStencil->SetOutputWholeExtent(binaryLabelmap->GetExtent());

  // Convert stencil to image
  vtkNew<vtkImageStencil> stencil;
  stencil->SetInputData(binaryLabelmap);
  stencil->SetStencilConnection(polyDataToImageStencil->GetOutputPort());
  stencil->ReverseStencilOn();
  stencil->SetBackgroundValue(labelValue);
  stencil->Update();

  vtkNew<vtkMRMLLabelMapVolumeNode> outputVolumeNode;
  outputVolumeNode->SetAndObserveImageData(stencil->GetOutput());
  outputVolumeNode->SetIJKToRASMatrix(ijkToRASMatrix);

  vtkNew<vtkMRMLVolumeArchetypeStorageNode> outputVolumeStorageNode;
  outputVolumeStorageNode->SetFileName(OutputVolume.c_str());
  if (!outputVolumeStorageNode->WriteData(outputVolumeNode))
    {
    std::cerr << "Failed to write output volume file " << OutputVolume << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
