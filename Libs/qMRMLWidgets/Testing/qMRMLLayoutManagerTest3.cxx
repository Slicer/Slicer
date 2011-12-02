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
#include <QWidget>

// SlicerQt includes
#include "qMRMLLayoutManager.h"

// MRML includes
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLLayoutNode.h>

// VTK includes
#include <vtkCollection.h>

// STD includes

int qMRMLLayoutManagerTest3(int argc, char * argv[] )
{
  QApplication app(argc, argv);
  QWidget w;
  w.show();
  qMRMLLayoutManager* layoutManager = new qMRMLLayoutManager(&w, &w);

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

  vtkMRMLLayoutLogic* layoutLogic = layoutManager->layoutLogic();
  vtkCollection* viewNodes = layoutLogic->GetViewNodes();
  if (viewNodes->GetNumberOfItems() != 1)
    {
    std::cerr << __LINE__ << " vtkMRMLLayoutLogic::GetViewNodes failed. "
              << "It found " << viewNodes->GetNumberOfItems() << " nodes instead of 1"
              << std::endl;
    return EXIT_FAILURE;
    }

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);

  viewNodes = layoutLogic->GetViewNodes();

  if (viewNodes->GetNumberOfItems() != 1)
    {
    std::cerr << __LINE__ << " vtkMRMLLayoutLogic::GetViewNodes failed. "
              << "It found " << viewNodes->GetNumberOfItems() << " nodes instead of 1"
              << std::endl;
    return EXIT_FAILURE;
    }

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutConventionalView);

  viewNodes = layoutLogic->GetViewNodes();

  if (viewNodes->GetNumberOfItems() != 4)
    {
    std::cerr << __LINE__ << " vtkMRMLLayoutLogic::GetViewNodes failed. "
              << "It found " << viewNodes->GetNumberOfItems() << " nodes instead of 4"
              << std::endl;
    return EXIT_FAILURE;
    }

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

