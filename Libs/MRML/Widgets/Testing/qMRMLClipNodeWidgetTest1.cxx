/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRML includes
#include "qMRMLClipNodeWidget.h"

// MRML includes
#include <vtkMRMLClipModelsNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include "qMRMLWidget.h"

// STD includes
#include <iostream>

int qMRMLClipNodeWidgetTest1(int argc, char* argv[])
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkNew<vtkMRMLScene> scene;

  vtkSmartPointer<vtkMRMLClipModelsNode> clipNode = vtkSmartPointer<vtkMRMLClipModelsNode>::New();
  scene->AddNode(clipNode);

  vtkSmartPointer<vtkMRMLSliceNode> redSlice = vtkSmartPointer<vtkMRMLSliceNode>::New();
  redSlice->SetSingletonTag("Red");
  scene->AddNode(redSlice);

  vtkSmartPointer<vtkMRMLSliceNode> yellowSlice = vtkSmartPointer<vtkMRMLSliceNode>::New();
  yellowSlice->SetSingletonTag("Yellow");
  scene->AddNode(yellowSlice);

  vtkSmartPointer<vtkMRMLSliceNode> greenSlice = vtkSmartPointer<vtkMRMLSliceNode>::New();
  greenSlice->SetSingletonTag("Green");
  scene->AddNode(greenSlice);

  qMRMLClipNodeWidget clipNodeWidget;
  clipNodeWidget.setMRMLScene(scene);

  if (clipNodeWidget.isEnabled())
  {
    std::cerr << "No vtkMRLMClipModelsNode provided, should be disabled." << std::endl;
    return EXIT_FAILURE;
  }

  int clipType = clipNode->GetClipType();
  int redSliceClipState = clipNode->GetRedSliceClipState();
  int yellowSliceClipState = clipNode->GetYellowSliceClipState();
  int greenSliceClipState = clipNode->GetGreenSliceClipState();

  clipNodeWidget.setMRMLClipNode(clipNode);

  if (clipNodeWidget.mrmlClipNode() != clipNode.GetPointer())
  {
    std::cerr << "qMRMLClipNodeWidget::setMRMLClipNode() failed." << clipNodeWidget.mrmlClipNode() << std::endl;
    return EXIT_FAILURE;
  }

  if (clipNodeWidget.clipType() != clipType)
  {
    std::cerr << "Wrong clipType: " << clipNodeWidget.clipType() << std::endl;
    return EXIT_FAILURE;
  }

  if (clipNodeWidget.clipState(redSlice) != redSliceClipState)
  {
    std::cerr << "Wrong red slice clip state: " << clipNodeWidget.clipState(redSlice) << std::endl;
    return EXIT_FAILURE;
  }

  if (clipNodeWidget.clipState(yellowSlice) != yellowSliceClipState)
  {
    std::cerr << "Wrong yellow slice clip state: " << clipNodeWidget.clipState(yellowSlice) << std::endl;
    return EXIT_FAILURE;
  }

  if (clipNodeWidget.clipState(greenSlice) != greenSliceClipState)
  {
    std::cerr << "Wrong green slice clip state: " << clipNodeWidget.clipState(greenSlice) << std::endl;
    return EXIT_FAILURE;
  }

  clipNode->SetClipType(vtkMRMLClipModelsNode::ClipIntersection);

  if (clipNodeWidget.clipType() != vtkMRMLClipModelsNode::ClipIntersection)
  {
    std::cerr << "vtkMRMLClipModelsNode::SetClipType() failed: " << clipNodeWidget.clipType() << std::endl;
    return EXIT_FAILURE;
  }

  clipNodeWidget.setClipType(vtkMRMLClipModelsNode::ClipUnion);

  if (clipNode->GetClipType() != vtkMRMLClipModelsNode::ClipUnion)
  {
    std::cerr << "qMRMLClipNodeWidget::setClipType() failed: " << clipNode->GetClipType() << std::endl;
    return EXIT_FAILURE;
  }

  // Red slice Clip state
  clipNode->SetRedSliceClipState(vtkMRMLClipModelsNode::ClipNegativeSpace);

  if (clipNodeWidget.clipState(redSlice) != vtkMRMLClipModelsNode::ClipNegativeSpace)
  {
    std::cerr << "vtkMRMLClipModelsNode::SetRedSliceClipState() failed: " << clipNodeWidget.clipState(redSlice) << std::endl;
    return EXIT_FAILURE;
  }

  clipNodeWidget.setClipState(redSlice, vtkMRMLClipModelsNode::ClipOff);

  if (clipNode->GetRedSliceClipState() != vtkMRMLClipModelsNode::ClipOff)
  {
    std::cerr << "qMRMLClipNodeWidget::setRedSliceClipState() failed: " << clipNode->GetRedSliceClipState() << std::endl;
    return EXIT_FAILURE;
  }

  // Yellow slice Clip state
  clipNode->SetYellowSliceClipState(vtkMRMLClipModelsNode::ClipOff);

  if (clipNodeWidget.clipState(yellowSlice) != vtkMRMLClipModelsNode::ClipOff)
  {
    std::cerr << "vtkMRMLClipModelsNode::SetYellowSliceClipState() failed: " << clipNodeWidget.clipState(yellowSlice) << std::endl;
    return EXIT_FAILURE;
  }

  clipNodeWidget.setClipState(yellowSlice, vtkMRMLClipModelsNode::ClipPositiveSpace);

  if (clipNode->GetYellowSliceClipState() != vtkMRMLClipModelsNode::ClipPositiveSpace)
  {
    std::cerr << "qMRMLClipNodeWidget::setYellowSliceClipState() failed: " << clipNode->GetYellowSliceClipState() << std::endl;
    return EXIT_FAILURE;
  }

  // Green slice Clip state
  clipNode->SetGreenSliceClipState(vtkMRMLClipModelsNode::ClipPositiveSpace);

  if (clipNodeWidget.clipState(greenSlice) != vtkMRMLClipModelsNode::ClipPositiveSpace)
  {
    std::cerr << "vtkMRMLClipModelsNode::SetGreenSliceClipState() failed: " << clipNodeWidget.clipState(greenSlice) << std::endl;
    return EXIT_FAILURE;
  }

  clipNodeWidget.setClipState(greenSlice, vtkMRMLClipModelsNode::ClipNegativeSpace);

  if (clipNode->GetGreenSliceClipState() != vtkMRMLClipModelsNode::ClipNegativeSpace)
  {
    std::cerr << "qMRMLClipNodeWidget::setGreenSliceClipState() failed: " << clipNode->GetGreenSliceClipState() << std::endl;
    return EXIT_FAILURE;
  }

  clipNodeWidget.show();

  if (argc < 2 || QString(argv[1]) != "-I")
  {
    QTimer::singleShot(200, &app, SLOT(quit()));
  }
  return app.exec();
}
