/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QWidget>

// SlicerQt includes
#include "qMRMLLayoutManager.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// STD includes
#include <cstdlib>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

int qMRMLLayoutManagerTest3(int argc, char * argv[] )
{
  QApplication app(argc, argv);
  QWidget w;
  w.show();
  qMRMLLayoutManager* layoutManager = new qMRMLLayoutManager(&w);

  vtkMRMLScene* scene = vtkMRMLScene::New();

  vtkMRMLLayoutNode* layoutNode = vtkMRMLLayoutNode::New();
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
  scene->AddNode(layoutNode);
  layoutNode->Delete();

  layoutManager->setMRMLScene(scene);

  if (layoutManager->layout() != vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView)
    {
    std::cerr << __LINE__ << " Add scene failed:" << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    return EXIT_FAILURE;
    }

  vtkCollection* sliceNodes = layoutNode->GetVisibleSliceViewNodes();
  vtkCollection* threeDViewNodes = layoutNode->GetVisibleThreeDViewNodes();
  vtkCollection* viewNodes = layoutNode->GetVisibleViewNodes();
  if (sliceNodes->GetNumberOfItems() != 1 ||
      threeDViewNodes->GetNumberOfItems() != 0 ||
      viewNodes->GetNumberOfItems() != 1)
    {
    std::cerr << __LINE__ << " vtkMRMLLayoutNode::GetVisibleSliceViewNodes failed. "
              << "It found " << sliceNodes->GetNumberOfItems() << " nodes instead of 1"
              << std::endl;
    return EXIT_FAILURE;
    }

  sliceNodes->Delete();
  threeDViewNodes->Delete();
  viewNodes->Delete();

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);

  sliceNodes = layoutNode->GetVisibleSliceViewNodes();
  threeDViewNodes = layoutNode->GetVisibleThreeDViewNodes();
  viewNodes = layoutNode->GetVisibleViewNodes();

  if (sliceNodes->GetNumberOfItems() != 1 ||
      threeDViewNodes->GetNumberOfItems() != 0 ||
      viewNodes->GetNumberOfItems() != 1)
    {
    std::cerr << __LINE__ << " vtkMRMLLayoutNode::GetVisibleSliceViewNodes failed. "
              << "It found " << sliceNodes->GetNumberOfItems() << " nodes instead of 1"
              << std::endl;
    return EXIT_FAILURE;
    }

  sliceNodes->Delete();
  threeDViewNodes->Delete();
  viewNodes->Delete();

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutConventionalView);

  sliceNodes = layoutNode->GetVisibleSliceViewNodes();
  threeDViewNodes = layoutNode->GetVisibleThreeDViewNodes();
  viewNodes = layoutNode->GetVisibleViewNodes();

  if (sliceNodes->GetNumberOfItems() != 3 ||
      threeDViewNodes->GetNumberOfItems() != 1 ||
      viewNodes->GetNumberOfItems() != 4)
    {
    std::cerr << __LINE__ << " vtkMRMLLayoutNode::GetVisibleSliceViewNodes failed. "
              << "It found " << sliceNodes->GetNumberOfItems() << " nodes instead of 3"
              << std::endl;
    return EXIT_FAILURE;
    }

  sliceNodes->Delete();
  threeDViewNodes->Delete();
  viewNodes->Delete();

  QTimer autoExit;
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QObject::connect(&autoExit, SIGNAL(timeout()), &app, SLOT(quit()));
    autoExit.start(1000);
    }
  int res = app.exec();

  scene->Delete();
  delete layoutManager;
  return res;
}

