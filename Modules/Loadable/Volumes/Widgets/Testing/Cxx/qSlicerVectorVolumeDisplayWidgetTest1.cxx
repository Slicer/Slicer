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

// Qt includes
#include <QApplication>
#include <QTimer>

// CTK includes
#include <ctkCollapsibleGroupBox.h>

// Volumes includes
#include "qSlicerVectorVolumeDisplayWidget.h"

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLVectorFieldDisplayNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLVectorVolumeNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include "qMRMLWidget.h"

//-----------------------------------------------------------------------------
int qSlicerVectorVolumeDisplayWidgetTest1(int argc, char* argv[])
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkNew<vtkMRMLScene> scene;
  vtkMRMLVectorVolumeNode* volumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode"));
  vtkMRMLVectorVolumeDisplayNode* displayNode = vtkMRMLVectorVolumeDisplayNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeDisplayNode"));
  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  // RGBA color volume
  vtkNew<vtkImageData> imageData;
  imageData->SetDimensions(4, 3, 2);
  imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 4);
  volumeNode->SetAndObserveImageData(imageData);

  qSlicerVectorVolumeDisplayWidget widget;
  widget.setMRMLScene(scene);
  widget.setMRMLVolumeNode(volumeNode);

  // Showing the volume does not create a vector field display node
  CHECK_NULL(widget.vectorFieldDisplayNode());

  // Opening the vector field section creates the display node, but it does not show the vector field
  ctkCollapsibleGroupBox* vectorFieldGroupBox = widget.findChild<ctkCollapsibleGroupBox*>("VectorFieldGroupBox");
  CHECK_NOT_NULL(vectorFieldGroupBox);
  vectorFieldGroupBox->setCollapsed(false);
  vtkMRMLVectorFieldDisplayNode* vectorFieldDisplayNode = widget.vectorFieldDisplayNode();
  CHECK_NOT_NULL(vectorFieldDisplayNode);
  CHECK_BOOL(vectorFieldDisplayNode->GetVisibility(), false);

  // Reopening the section uses the existing display node and keeps its visibility
  vectorFieldDisplayNode->SetVisibility(true);
  vectorFieldGroupBox->setCollapsed(true);
  vectorFieldGroupBox->setCollapsed(false);
  CHECK_POINTER(widget.vectorFieldDisplayNode(), vectorFieldDisplayNode);
  CHECK_BOOL(vectorFieldDisplayNode->GetVisibility(), true);

  widget.show();
  if (argc < 2 || QString(argv[1]) != "-I")
  {
    QTimer::singleShot(200, &app, SLOT(quit()));
  }
  return app.exec();
}
