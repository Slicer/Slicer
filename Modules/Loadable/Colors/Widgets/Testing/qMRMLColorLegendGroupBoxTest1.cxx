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

// Checks that the color legend section shows the given display node's legend, creates one (hidden) only when expanded,
// and is only enabled while the display node colors by scalars through a color table.

// Qt includes
#include <QApplication>

// Colors widgets includes
#include "qMRMLColorLegendDisplayNodeWidget.h"
#include "qMRMLColorLegendGroupBox.h"

// Colors logic includes
#include <vtkSlicerColorLogic.h>

// MRML includes
#include <vtkMRMLColorLegendDisplayNode.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>

// qMRML includes
#include "qMRMLWidget.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
int qMRMLColorLegendGroupBoxTest1(int argc, char* argv[])
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkNew<vtkMRMLScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLColorLegendDisplayNode>::New());
  vtkMRMLColorTableNode* colorNode = vtkMRMLColorTableNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLColorTableNode"));
  colorNode->SetTypeToCool1();
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelDisplayNode"));
  modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());
  displayNode->SetScalarVisibility(true);

  qMRMLColorLegendGroupBox groupBox;
  CHECK_BOOL(groupBox.isEnabled(), false);
  CHECK_BOOL(groupBox.collapsed(), true);

  // Shown for the display node, without adding a legend yet
  groupBox.setPrimaryDisplayNode(displayNode);
  CHECK_POINTER(groupBox.primaryDisplayNode(), displayNode);
  CHECK_BOOL(groupBox.isEnabled(), true);
  CHECK_NULL(vtkSlicerColorLogic::GetColorLegendDisplayNode(displayNode));
  CHECK_NULL(groupBox.colorLegendDisplayNodeWidget()->mrmlColorLegendDisplayNode());

  // Only enabled while the colors come from a color table
  displayNode->SetScalarVisibility(false);
  CHECK_BOOL(groupBox.isEnabled(), false);
  displayNode->SetScalarVisibility(true);
  CHECK_BOOL(groupBox.isEnabled(), true);

  // Expanding it adds the legend, hidden until the user shows it
  groupBox.setCollapsed(false);
  vtkMRMLColorLegendDisplayNode* colorLegendNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(displayNode);
  CHECK_NOT_NULL(colorLegendNode);
  CHECK_BOOL(colorLegendNode->GetVisibility(), false);
  CHECK_POINTER(colorLegendNode->GetPrimaryDisplayNode(), displayNode);
  CHECK_POINTER(groupBox.colorLegendDisplayNodeWidget()->mrmlColorLegendDisplayNode(), colorLegendNode);

  // Another section for the same display node shows the same legend, and does not add one
  qMRMLColorLegendGroupBox otherGroupBox;
  otherGroupBox.setPrimaryDisplayNode(displayNode);
  CHECK_POINTER(otherGroupBox.colorLegendDisplayNodeWidget()->mrmlColorLegendDisplayNode(), colorLegendNode);
  CHECK_INT(vtkSlicerColorLogic::GetNumberOfColorLegendDisplayNodes(modelNode), 1);

  // Without a display node there is nothing to show
  groupBox.setPrimaryDisplayNode(static_cast<vtkMRMLDisplayNode*>(nullptr));
  CHECK_BOOL(groupBox.isEnabled(), false);
  CHECK_NULL(groupBox.colorLegendDisplayNodeWidget()->mrmlColorLegendDisplayNode());

  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
