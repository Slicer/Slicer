/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// Logic includes
#include "vtkSlicerTransformLogic.h"

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLTransformDisplayNode.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>

// STD includes
#include <iostream>

//----------------------------------------------------------------------------
int vtkSlicerTransformLogicVisualizationTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  vtkNew<vtkMRMLScene> scene;
  // A rotation and a translation, so that the displacement varies from place to place and
  // has contours
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode"));
  vtkNew<vtkTransform> transform;
  transform->Translate(3.0, 0.0, 0.0);
  transform->RotateZ(10.0);
  transformNode->SetMatrixTransformToParent(transform->GetMatrix());
  transformNode->CreateDefaultDisplayNodes();
  vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
  CHECK_NOT_NULL(displayNode);
  double levels[1] = { 5.0 };
  displayNode->SetContourLevels(levels, 1);
  displayNode->SetGridSpacing(10.0);

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSliceNode"));
  sliceNode->SetDimensions(256, 256, 1);
  sliceNode->SetFieldOfView(100.0, 100.0, 1.0);
  // The default slice to RAS is axial
  sliceNode->UpdateMatrices();

  // A box of 60 mm around the origin
  vtkNew<vtkMatrix4x4> roiToRAS;
  for (int axis = 0; axis < 3; ++axis)
  {
    roiToRAS->SetElement(axis, 3, -30.0);
  }
  int roiSize[3] = { 60, 60, 60 };

  // A sampling spacing of 0 follows the resolution of the field, which a transform does not
  // have: it must not be divided by
  for (double spacingMm : { 5.0, 0.0 })
  {
    displayNode->SetSamplingSpacing(spacingMm);
    for (int mode : { vtkMRMLTransformDisplayNode::VIS_MODE_GLYPH, vtkMRMLTransformDisplayNode::VIS_MODE_GRID, vtkMRMLTransformDisplayNode::VIS_MODE_CONTOUR })
    {
      displayNode->SetVisualizationMode(mode);
      vtkNew<vtkPolyData> output2d;
      CHECK_BOOL(vtkSlicerTransformLogic::GetVisualization2d(output2d, displayNode, sliceNode), true);
      vtkNew<vtkPolyData> output3d;
      CHECK_BOOL(vtkSlicerTransformLogic::GetVisualization3d(output3d, displayNode, roiToRAS, roiSize), true);
      std::cout << "Spacing " << spacingMm << ", " << displayNode->GetVisualizationModeAsString() << ": " << output2d->GetNumberOfPoints() << " points in 2D, "
                << output3d->GetNumberOfPoints() << " in 3D" << std::endl;
      CHECK_BOOL(output2d->GetNumberOfPoints() > 0, true);
      CHECK_BOOL(output3d->GetNumberOfPoints() > 0, true);
    }
  }
  return EXIT_SUCCESS;
}
