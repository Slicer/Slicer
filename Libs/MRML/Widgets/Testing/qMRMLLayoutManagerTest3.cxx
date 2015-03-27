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

// Slicer includes
#include "qMRMLLayoutManager.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

// Common test driver includes
#include "qMRMLWidgetCxxTests.h"

namespace
{

// --------------------------------------------------------------------------
bool checkViewArrangement(int line, qMRMLLayoutManager* layoutManager,
                          vtkMRMLLayoutNode * layoutNode, int expectedViewArrangement)
{
  if (layoutManager->layout() != expectedViewArrangement ||
      layoutNode->GetViewArrangement() != expectedViewArrangement)
    {
    std::cerr << "Line " << line << " - Add scene failed:\n"
              << " expected ViewArrangement: " << expectedViewArrangement << "\n"
              << " current ViewArrangement: " << layoutNode->GetViewArrangement() << "\n"
              << " current layout: " << layoutManager->layout() << std::endl;
    return false;
    }
  return true;
}

// --------------------------------------------------------------------------
bool checkNumberOfItems(int line, qMRMLLayoutManager* layoutManager, int expected)
{
  vtkMRMLLayoutLogic * layoutLogic = layoutManager->layoutLogic();
  vtkCollection* viewNodes = layoutLogic->GetViewNodes();
  if (viewNodes->GetNumberOfItems() != expected)
    {
    std::cerr << "Line " << line << " - Problem with vtkMRMLLayoutLogic::GetViewNodes()\n"
              << " expected NumberOfItems: " << expected << "\n"
              << " current NumberOfItems: " << viewNodes->GetNumberOfItems() << std::endl;
    return false;
    }
  return true;
}

} // end of anonymous namespace

// --------------------------------------------------------------------------
int qMRMLLayoutManagerTest3(int argc, char * argv[] )
{
  QApplication app(argc, argv);
  QWidget w;
  w.show();
  qMRMLLayoutManager* layoutManager = new qMRMLLayoutManager(&w, &w);

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLApplicationLogic> applicationLogic;

  vtkMRMLLayoutNode * layoutNode = 0;
  {
    vtkNew<vtkMRMLLayoutNode> newLayoutNode;

    // The view arrangement can be set before the view descriptions are registered
    // into the layout node. Setting the scene to the layout manager will set the
    // the scene to the layout logic which will register the layout descriptions.
    TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
    newLayoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
    TESTING_OUTPUT_ASSERT_WARNINGS_END();

    layoutNode = vtkMRMLLayoutNode::SafeDownCast(scene->AddNode(newLayoutNode.GetPointer()));
  }

  applicationLogic->SetMRMLScene(scene.GetPointer());
  layoutManager->setMRMLScene(scene.GetPointer());

  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView))
    {
    return EXIT_FAILURE;
    }
  if (!checkNumberOfItems(__LINE__, layoutManager, /* expected = */ 1))
    {
    return EXIT_FAILURE;
    }

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView))
    {
    return EXIT_FAILURE;
    }
  if (!checkNumberOfItems(__LINE__, layoutManager, /* expected = */ 1))
    {
    return EXIT_FAILURE;
    }

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutConventionalView);
  if (!checkViewArrangement(__LINE__, layoutManager, layoutNode, vtkMRMLLayoutNode::SlicerLayoutConventionalView))
    {
    return EXIT_FAILURE;
    }
  if (!checkNumberOfItems(__LINE__, layoutManager, /* expected = */ 4))
    {
    return EXIT_FAILURE;
    }

  QTimer autoExit;
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QObject::connect(&autoExit, SIGNAL(timeout()), &app, SLOT(quit()));
    autoExit.start(1000);
    }
  return app.exec();
}
