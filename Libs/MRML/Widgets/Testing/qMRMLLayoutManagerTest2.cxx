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
#include <QWidget>

// Slicer includes
#include "qMRMLLayoutManager.h"
#include "vtkSlicerConfigure.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// Common test driver includes
#include "qMRMLWidgetCxxTests.h"
#include "qMRMLLayoutManagerTestHelper.cxx"

// --------------------------------------------------------------------------
int qMRMLLayoutManagerTest2(int argc, char * argv[] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  QWidget w;
  w.show();
  qMRMLLayoutManager* layoutManager = new qMRMLLayoutManager(&w, &w);

  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->SetMRMLApplicationLogic(applicationLogic);

  {
    vtkNew<vtkMRMLScene> scene;
    applicationLogic->SetMRMLScene(scene.GetPointer());
    layoutManager->setMRMLScene(scene.GetPointer());
    if (layoutManager->mrmlScene() != scene.GetPointer())
      {
      std::cerr << __LINE__ << " Problem with setMRMLScene()" << std::endl;
      return EXIT_FAILURE;
      }

    vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();

    if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutInitialView))
      {
      return EXIT_FAILURE;
      }

    int expectedThreeDViewCout = 1;
    int currentThreeDViewCount = layoutManager->threeDViewCount();
    if (expectedThreeDViewCout != currentThreeDViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qMRMLLayoutManager\n"
                << "  expectedThreeDViewCout:" << expectedThreeDViewCout << "\n"
                << "  currentThreeDViewCount:" << currentThreeDViewCount << std::endl;
      return EXIT_FAILURE;
      }

    int expectedSliceViewCout = 3;
    int currentSliceViewCount = layoutManager->sliceViewNames().count();
    if (expectedSliceViewCout != currentSliceViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qMRMLLayoutManager\n"
                << "  expectedSliceViewCout:" << expectedSliceViewCout << "\n"
                << "  currentSliceViewCount:" << currentSliceViewCount << std::endl;
      return EXIT_FAILURE;
      }

    int expectedChartViewCout = 0;
    int currentChartViewCount = layoutManager->chartViewCount();
    if (expectedChartViewCout != currentChartViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qMRMLLayoutManager\n"
                << "  expectedChartViewCout:" << expectedChartViewCout << "\n"
                << "  currentChartViewCount:" << currentChartViewCount << std::endl;
      return EXIT_FAILURE;
      }

    int expectedTableViewCout = 0;
    int currentTableViewCount = layoutManager->tableViewCount();
    if (expectedTableViewCout != currentTableViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qMRMLLayoutManager\n"
                << "  expectedTableViewCout:" << expectedTableViewCout << "\n"
                << "  currentTableViewCount:" << currentTableViewCount << std::endl;
      return EXIT_FAILURE;
      }

    layoutManager->setMRMLScene(nullptr);
    applicationLogic->SetMRMLScene(nullptr);

    int current = scene->GetReferenceCount();
    int expected = 1;
    if (current != expected)
      {
      std::cerr << __LINE__ << " Problem with MRMLScene reference count !\n"
                << "  current: " << current << "\n"
                << "  expected: " << expected << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    // Setting a new scene is expected to reset the factories
    vtkNew<vtkMRMLScene> scene;
    applicationLogic->SetMRMLScene(scene.GetPointer());
    layoutManager->setMRMLScene(scene.GetPointer());
    vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();

    if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutInitialView))
      {
      return EXIT_FAILURE;
      }

    int expectedThreeDViewCout = 1;
    int currentThreeDViewCount = layoutManager->threeDViewCount();
    if (expectedThreeDViewCout != currentThreeDViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qMRMLLayoutManager\n"
                << "  expectedThreeDViewCout:" << expectedThreeDViewCout << "\n"
                << "  currentThreeDViewCount:" << currentThreeDViewCount << std::endl;
      return EXIT_FAILURE;
      }

    int expectedSliceViewCout = 3;
    int currentSliceViewCount = layoutManager->sliceViewNames().count();
    if (expectedSliceViewCout != currentSliceViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qMRMLLayoutManager\n"
                << "  expectedSliceViewCout:" << expectedSliceViewCout << "\n"
                << "  currentSliceViewCount:" << currentSliceViewCount << std::endl;
      return EXIT_FAILURE;
      }

    int expectedChartViewCout = 0;
    int currentChartViewCount = layoutManager->chartViewCount();
    if (expectedChartViewCout != currentChartViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qMRMLLayoutManager\n"
                << "  expectedChartViewCout:" << expectedChartViewCout << "\n"
                << "  currentChartViewCount:" << currentChartViewCount << std::endl;
      return EXIT_FAILURE;
      }

    int expectedTableViewCout = 0;
    int currentTableViewCount = layoutManager->tableViewCount();
    if (expectedTableViewCout != currentTableViewCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qMRMLLayoutManager\n"
                << "  expectedTableViewCout:" << expectedTableViewCout << "\n"
                << "  currentTableViewCount:" << currentTableViewCount << std::endl;
      return EXIT_FAILURE;
      }

  }
  vtkMRMLLayoutNode* layoutNode = nullptr;
  {
    vtkNew<vtkMRMLScene> scene;
    vtkNew<vtkMRMLLayoutNode> newLayoutNode;

    // The view arrangement can be set before the view descriptions are registered, but it will log warning
    TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
    newLayoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
    TESTING_OUTPUT_ASSERT_WARNINGS_END();

    layoutNode = vtkMRMLLayoutNode::SafeDownCast(scene->AddNode(newLayoutNode.GetPointer()));
    applicationLogic->SetMRMLScene(scene.GetPointer());
    layoutManager->setMRMLScene(scene.GetPointer());
  }

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView))
    {
    return EXIT_FAILURE;
    }

  layoutManager->setLayout(vtkMRMLLayoutNode::SlicerLayoutCompareView);
  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutCompareView))
    {
    return EXIT_FAILURE;
    }

  vtkMRMLScene * scene = layoutManager->mrmlScene();

  scene->StartState(vtkMRMLScene::ImportState);
  scene->EndState(vtkMRMLScene::ImportState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutCompareView))
    {
    return EXIT_FAILURE;
    }

  scene->StartState(vtkMRMLScene::ImportState);
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
  scene->EndState(vtkMRMLScene::ImportState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView))
    {
    return EXIT_FAILURE;
    }

  scene->StartState(vtkMRMLScene::CloseState);
  scene->EndState(vtkMRMLScene::CloseState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView))
    {
    return EXIT_FAILURE;
    }

  scene->StartState(vtkMRMLScene::CloseState);
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
  scene->EndState(vtkMRMLScene::CloseState);

  if (!checkViewArrangement(__LINE__, layoutManager,layoutNode,  vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }

  // The layout is changed to none only if vtkMRMLScene::Clear() is called
  scene->StartState(vtkMRMLScene::CloseState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }

  // Imitates what vtkMRMLScene::Clear() would have done:
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutNone);

  // and restore it back
  scene->EndState(vtkMRMLScene::CloseState);

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    return safeApplicationQuit(&app);
    }
  else
    {
    return app.exec();
    }
}

