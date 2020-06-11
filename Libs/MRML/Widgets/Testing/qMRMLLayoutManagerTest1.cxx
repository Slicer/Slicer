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

// Common test driver includes
#include "qMRMLWidgetCxxTests.h"
#include "qMRMLLayoutManagerTestHelper.cxx"

// Qt includes
#include <QApplication>
#include <QWidget>

// Slicer includes
#include <qMRMLWidgetsConfigure.h> // For MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
#include "qMRMLChartWidget.h"
#endif
#include "qMRMLLayoutManager.h"
#include "qMRMLSliceWidget.h"
#include "qMRMLTableWidget.h"
#include "qMRMLThreeDWidget.h"
#include "vtkSlicerConfigure.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLChartViewNode.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLTableViewNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

namespace
{
//------------------------------------------------------------------------------
bool testLayoutManagerViewWidgetForChart(int line, qMRMLLayoutManager* layoutManager, int viewId)
{
#ifdef MRML_WIDGETS_HAVE_WEBENGINE_SUPPORT
  qMRMLChartWidget* widget = layoutManager->chartWidget(viewId);
  vtkMRMLChartViewNode* node = widget ? widget->mrmlChartViewNode() : nullptr;
  if (!widget || !node)
    {
    std::cerr << "Line " << line << " - Problem with qMRMLLayoutManager::chartWidget()" << std::endl;
    return false;
    }
  if (layoutManager->viewWidget(node) != widget)
    {
    std::cerr << "Line " << line << " - Problem with qMRMLLayoutManager::viewWidget()" << std::endl;
    return false;
    }
#else
  Q_UNUSED(line);
  Q_UNUSED(layoutManager);
  Q_UNUSED(viewId);
#endif
  return true;
}
//------------------------------------------------------------------------------
bool testLayoutManagerViewWidgetForTable(int line, qMRMLLayoutManager* layoutManager, int viewId)
{
  qMRMLTableWidget* widget = layoutManager->tableWidget(viewId);
  vtkMRMLTableViewNode* node = widget ? widget->mrmlTableViewNode() : nullptr;
  if (!widget || !node)
    {
    std::cerr << "Line " << line << " - Problem with qMRMLLayoutManager::tableWidget()" << std::endl;
    return false;
    }
  if (layoutManager->viewWidget(node) != widget)
    {
    std::cerr << "Line " << line << " - Problem with qMRMLLayoutManager::viewWidget()" << std::endl;
    return false;
    }
  return true;
}
//------------------------------------------------------------------------------
bool testLayoutManagerViewWidgetForSlice(int line, qMRMLLayoutManager* layoutManager, const char* viewName)
{
  qMRMLSliceWidget* widget = layoutManager->sliceWidget(viewName);
  vtkMRMLSliceNode* node = widget ? widget->mrmlSliceNode() : nullptr;
  if (!widget || !node)
    {
    std::cerr << "Line " << line << " - Problem with qMRMLLayoutManager::sliceWidget()" << std::endl;
    return false;
    }
  if (layoutManager->viewWidget(node) != widget)
    {
    std::cerr << "Line " << line << " - Problem with qMRMLLayoutManager::viewWidget()" << std::endl;
    return false;
    }
  return true;
}
//------------------------------------------------------------------------------
bool testLayoutManagerViewWidgetForThreeD(int line, qMRMLLayoutManager* layoutManager, int viewId)
{
  qMRMLThreeDWidget* widget = layoutManager->threeDWidget(viewId);
  vtkMRMLViewNode* node = widget ? widget->mrmlViewNode() : nullptr;
  if (!widget || !node)
    {
    std::cerr << "Line " << line << " - Problem with qMRMLLayoutManager::threeDWidget()" << std::endl;
    return false;
    }
  if (layoutManager->viewWidget(node) != widget)
    {
    std::cerr << "Line " << line << " - Problem with qMRMLLayoutManager::viewWidget()" << std::endl;
    return false;
    }
  return true;
}
}

//------------------------------------------------------------------------------
int qMRMLLayoutManagerTest1(int argc, char * argv[] )
{
  (void)checkViewArrangement; // Fix -Wunused-function warning

  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();
  qMRMLLayoutManager * layoutManager = new qMRMLLayoutManager();

  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->SetMRMLApplicationLogic(applicationLogic);
  {
    vtkNew<vtkMRMLScene> scene;
    applicationLogic->SetMRMLScene(scene.GetPointer());
    layoutManager->setMRMLScene(scene.GetPointer());
    if (layoutManager->mrmlScene() != scene.GetPointer())
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qMRMLLayoutManager::setMRMLScene()" << std::endl;
      return EXIT_FAILURE;
      }
    layoutManager->setMRMLScene(nullptr);
    applicationLogic->SetMRMLScene(nullptr);
    if (layoutManager->mrmlScene() != nullptr)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qMRMLLayoutManager::setMRMLScene()" << std::endl;
      return EXIT_FAILURE;
      }
  }

  vtkNew<vtkMRMLScene> scene;
  applicationLogic->SetMRMLScene(scene.GetPointer());
  layoutManager->setMRMLScene(scene.GetPointer());

  QWidget * viewport = new QWidget;
  viewport->setWindowTitle("Old widget");
  layoutManager->setViewport(viewport);
  viewport->show();
  layoutManager->setViewport(nullptr);
  layoutManager->setViewport(viewport);

  QWidget * viewport2 = new QWidget;
  viewport2->setWindowTitle("New widget");
  layoutManager->setViewport(viewport2);
  viewport2->show();


  layoutManager->setLayout(vtkMRMLLayoutNode::SlicerLayoutConventionalView);
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Green"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Red"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Yellow"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForThreeD(__LINE__, layoutManager, 0))
    {
    return EXIT_FAILURE;
    }

  layoutManager->setLayout(vtkMRMLLayoutNode::SlicerLayoutFourUpQuantitativeView);
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Green"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Red"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Yellow"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForChart(__LINE__, layoutManager, 0))
    {
    return EXIT_FAILURE;
    }

  layoutManager->setLayout(vtkMRMLLayoutNode::SlicerLayoutFourUpTableView);
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Green"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Red"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForSlice(__LINE__, layoutManager, "Yellow"))
    {
    return EXIT_FAILURE;
    }
  if (!testLayoutManagerViewWidgetForTable(__LINE__, layoutManager, 0))
    {
    return EXIT_FAILURE;
    }

  int res = 0;
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    res = safeApplicationQuit(&app);
    }
  else
    {
    res = app.exec();
    }

  delete layoutManager;
  delete viewport;
  delete viewport2;
  return res;
}

