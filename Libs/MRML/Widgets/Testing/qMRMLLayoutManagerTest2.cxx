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
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>

// VTK includes

// STD includes

int qMRMLLayoutManagerTest2(int argc, char * argv[] )
{
  QApplication app(argc, argv);
  QWidget w;
  w.show();
  qMRMLLayoutManager* layoutManager = new qMRMLLayoutManager(&w, &w);

  vtkMRMLScene* scene = vtkMRMLScene::New();
  layoutManager->setMRMLScene(scene);
  if (layoutManager->mrmlScene() != scene)
    {
    std::cerr << "scene incorrectly set." << std::endl;
    return EXIT_FAILURE;
    }
  layoutManager->setMRMLScene(0);
  scene->Delete();
  scene = vtkMRMLScene::New();

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
    scene->Delete();
    return EXIT_FAILURE;
    }

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
  if (layoutManager->layout() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView)
    {
    std::cerr << __LINE__ << " Set View Arrangement on LayoutNode failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    scene->Delete();
    return EXIT_FAILURE;
    }

  layoutManager->setLayout(vtkMRMLLayoutNode::SlicerLayoutCompareView);
  if (layoutManager->layout() != vtkMRMLLayoutNode::SlicerLayoutCompareView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutCompareView)
    {
    std::cerr << __LINE__ << " Set View Arrangement on layout manager failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutCompareView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    scene->Delete();
    return EXIT_FAILURE;
    }
  scene->StartState(vtkMRMLScene::ImportState);
  scene->EndState(vtkMRMLScene::ImportState);

  if (layoutManager->layout() != vtkMRMLLayoutNode::SlicerLayoutCompareView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutCompareView)
    {
    std::cerr << __LINE__ <<  " Set View Arrangement on layout manager failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutCompareView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    scene->Delete();
    return EXIT_FAILURE;
    }
  scene->StartState(vtkMRMLScene::ImportState);
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
  scene->EndState(vtkMRMLScene::ImportState);

  if (layoutManager->layout() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView)
    {
    std::cerr << __LINE__ << "Set View Arrangement on layout manager failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    scene->Delete();
    return EXIT_FAILURE;
    }

  scene->StartState(vtkMRMLScene::CloseState);
  scene->EndState(vtkMRMLScene::CloseState);

  if (layoutManager->layout() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView)
    {
    std::cerr << __LINE__ << "Set View Arrangement on layout manager failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    scene->Delete();
    return EXIT_FAILURE;
    }

  scene->StartState(vtkMRMLScene::CloseState);
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
  scene->EndState(vtkMRMLScene::CloseState);

  if (layoutManager->layout() != vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView)
    {
    std::cerr << __LINE__ << "Set View Arrangement on layout manager failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    scene->Delete();
    return EXIT_FAILURE;
    }

  // The layout is changed to none only if vtkMRMLScene::Clear() is called
  scene->StartState(vtkMRMLScene::CloseState);

  if (layoutManager->layout() != vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView)
    {
    std::cerr << __LINE__ << "Set View Arrangement on layout manager failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    scene->Delete();
    return EXIT_FAILURE;
    }
  // Imitates what vtkMRMLScene::Clear() would have done:
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutNone);

  // and restore it back
  scene->EndState(vtkMRMLScene::CloseState);

  if (layoutManager->layout() != vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView)
    {
    std::cerr << __LINE__ << "Set View Arrangement on layout manager failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    scene->Delete();
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

