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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QHash>
#include <QWidget>

// Slicer includes
#include "qMRMLLayoutManager.h"

// MRML includes
#include <vtkMRMLAbstractViewNode.h>
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkNew.h>

// --------------------------------------------------------------------------
bool checkNodeVisibility(int line,
                         vtkMRMLAbstractViewNode* viewNode,
                         bool expectedNodeVisibility)
{
  int nodeVisibility = viewNode->GetVisibility();
  if (nodeVisibility != expectedNodeVisibility)
    {
    std::cerr << "Line " << line << " - Problem with GetVisibility()"
              << " associated with node " << viewNode->GetID() << "\n"
              << "  visibility: " << nodeVisibility << "\n"
              << "  expectedVisibility: " << expectedNodeVisibility << std::endl;
    return false;
    }
  return true;
}

// --------------------------------------------------------------------------
bool checkViews(int line,
                qMRMLLayoutManager* layoutManager,
                QHash<vtkMRMLAbstractViewNode*, bool> viewNodesToExpectedVisibility)
{
  foreach(vtkMRMLAbstractViewNode* viewNode, viewNodesToExpectedVisibility.keys())
    {
    if (!viewNode)
      {
      std::cerr << "Line " << line << " - Problem with to GetNodeByID()."
               << " 'viewNode' should NOT be null." << std::endl;
      return false;
      }

    if (!checkNodeVisibility(line, viewNode, viewNodesToExpectedVisibility[viewNode]))
      {
      return false;
      }

    QWidget* viewWidget = layoutManager->viewWidget(viewNode);
    if (!viewWidget)
      {
      std::cerr << "Line " << line << " - Failed to retrieve view widget"
               << "associated with view node " << viewNode->GetID() << std::endl;
      return false;
      }

    bool widgetVisibility = viewWidget->isVisible();
    bool expectedWidgetVisibility = viewNodesToExpectedVisibility[viewNode];
    if (widgetVisibility != expectedWidgetVisibility)
      {
      std::cerr << "Line " << line << " - Problem with widget visibility"
                << " associated with node " << viewNode->GetID() << "\n"
                << "  widgetVisibility: " << widgetVisibility << "\n"
                << "  expectedWidgetVisibility: " << expectedWidgetVisibility << std::endl;
      return false;
      }
    }
  return true;
}

// --------------------------------------------------------------------------
bool runTests(vtkMRMLScene* scene,
              vtkMRMLLayoutNode* layoutNode,
              qMRMLLayoutManager* layoutManager)
{
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutConventionalView);
  qApp->processEvents();

  vtkMRMLAbstractViewNode* redNode = vtkMRMLAbstractViewNode::SafeDownCast(
        scene->GetNodeByID("vtkMRMLSliceNodeRed"));
  vtkMRMLAbstractViewNode* yellowNode = vtkMRMLAbstractViewNode::SafeDownCast(
        scene->GetNodeByID("vtkMRMLSliceNodeYellow"));
  vtkMRMLAbstractViewNode* greenNode = vtkMRMLAbstractViewNode::SafeDownCast(
        scene->GetNodeByID("vtkMRMLSliceNodeGreen"));
  vtkMRMLAbstractViewNode* threeDNode = vtkMRMLAbstractViewNode::SafeDownCast(
        scene->GetNodeByID("vtkMRMLViewNode1"));


  // All view widgets are expected to be visible
  {
    QHash<vtkMRMLAbstractViewNode*, bool> viewNodesToExpectedVisibility;
    viewNodesToExpectedVisibility[redNode] = true;
    viewNodesToExpectedVisibility[yellowNode] = true;
    viewNodesToExpectedVisibility[greenNode] = true;
    viewNodesToExpectedVisibility[threeDNode] = true;

    if (!checkViews(__LINE__, layoutManager, viewNodesToExpectedVisibility))
      {
      return false;
      }
  }

  yellowNode->SetVisibility(0);
  qApp->processEvents();

  // Yellow widget is expected to be hidden
  {
    QHash<vtkMRMLAbstractViewNode*, bool> viewNodesToExpectedVisibility;
    viewNodesToExpectedVisibility[redNode] = true;
    viewNodesToExpectedVisibility[yellowNode] = false;
    viewNodesToExpectedVisibility[greenNode] = true;
    viewNodesToExpectedVisibility[threeDNode] = true;

    if (!checkViews(__LINE__, layoutManager, viewNodesToExpectedVisibility))
      {
      return false;
      }
  }

  threeDNode->SetVisibility(0);
  qApp->processEvents();

  // Yellow and ThreeD widgets are expected to be hidden
  {
    QHash<vtkMRMLAbstractViewNode*, bool> viewNodesToExpectedVisibility;
    viewNodesToExpectedVisibility[redNode] = true;
    viewNodesToExpectedVisibility[yellowNode] = false;
    viewNodesToExpectedVisibility[greenNode] = true;
    viewNodesToExpectedVisibility[threeDNode] = false;

    if (!checkViews(__LINE__, layoutManager, viewNodesToExpectedVisibility))
      {
      return false;
      }
  }

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutFourUpQuantitativeView);
  qApp->processEvents();

  vtkMRMLAbstractViewNode* chartNode =
      vtkMRMLAbstractViewNode::SafeDownCast(scene->GetNodeByID("vtkMRMLChartViewNodeChartView1"));

  // Only yellow widgets is expected to be hidden
  {
    QHash<vtkMRMLAbstractViewNode*, bool> viewNodesToExpectedVisibility;
    viewNodesToExpectedVisibility[redNode] = true;
    viewNodesToExpectedVisibility[yellowNode] = false;
    viewNodesToExpectedVisibility[greenNode] = true;
    viewNodesToExpectedVisibility[chartNode] = true;

    if (!checkViews(__LINE__, layoutManager, viewNodesToExpectedVisibility))
      {
      return false;
      }
  }

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutConventionalView);
  qApp->processEvents();

  // Yellow and ThreeD widgets are expected to be hidden
  {
    QHash<vtkMRMLAbstractViewNode*, bool> viewNodesToExpectedVisibility;
    viewNodesToExpectedVisibility[redNode] = true;
    viewNodesToExpectedVisibility[yellowNode] = false;
    viewNodesToExpectedVisibility[greenNode] = true;
    viewNodesToExpectedVisibility[threeDNode] = false;

    if (!checkViews(__LINE__, layoutManager, viewNodesToExpectedVisibility))
      {
      return false;
      }
  }

  scene->Clear(/*removeSingletons = */ 0);
  qApp->processEvents();

  // All view widgets are expected to be visible
  {
    QHash<vtkMRMLAbstractViewNode*, bool> viewNodesToExpectedVisibility;
    viewNodesToExpectedVisibility[redNode] = true;
    viewNodesToExpectedVisibility[yellowNode] = true;
    viewNodesToExpectedVisibility[greenNode] = true;
    viewNodesToExpectedVisibility[threeDNode] = true;

    if (!checkViews(__LINE__, layoutManager, viewNodesToExpectedVisibility))
      {
      return false;
      }
  }

  {
    // The following test will check that the view node visibility attribute
    // is ignored when loading a scene.
    // For more detailed, see comment in vtkMRMLAbstractViewNode::ReadXMLAttributes

    yellowNode->SetVisibility(0);
    qApp->processEvents();

    if (!checkNodeVisibility(__LINE__,
                             yellowNode,
                             /* expectedNodeVisibility = */ 0))
      {
      return false;
      }

    scene->SetSaveToXMLString(1);
    scene->Commit();

    // Serialized scene has yellow node with visibility set to 0
    std::string xmlScene = scene->GetSceneXMLString();

    // Clear current scene
    scene->Clear(/*removeSingletons = */ 0);
    qApp->processEvents();

    if (!checkNodeVisibility(__LINE__,
                             yellowNode,
                             /* expectedNodeVisibility = */ 1))
      {
      return false;
      }

    scene->SetLoadFromXMLString(1);
    scene->SetSceneXMLString(xmlScene);
    scene->Import();
    qApp->processEvents();

    if (!checkNodeVisibility(__LINE__,
                             yellowNode,
                             /* expectedNodeVisibility = */ 1))
      {
      return false;
      }
  }

  return true;
}

// --------------------------------------------------------------------------
int qMRMLLayoutManagerVisibilityTest(int argc, char * argv[] )
{
  QApplication app(argc, argv);

  QWidget w;
  w.show();

  qMRMLLayoutManager layoutManager(&w, &w);

  vtkNew<vtkMRMLApplicationLogic> applicationLogic;

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLLayoutNode> layoutNode;

  scene->AddNode(layoutNode.GetPointer());
  applicationLogic->SetMRMLScene(scene.GetPointer());
  layoutManager.setMRMLScene(scene.GetPointer());

  if (!runTests(scene.GetPointer(), layoutNode.GetPointer(), &layoutManager))
    {
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
