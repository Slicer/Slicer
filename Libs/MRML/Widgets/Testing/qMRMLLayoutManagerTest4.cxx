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

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>

// Common test driver includes
#include "qMRMLLayoutManagerTestHelper.cxx"

int qMRMLLayoutManagerTest4(int argc, char * argv[] )
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

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);

  for (int i = vtkMRMLLayoutNode::SlicerLayoutInitialView;
    i < vtkMRMLLayoutNode::SlicerLayoutFinalView-1; ++i)
    {
    layoutManager.setLayout(i);
    if (!checkViewArrangement(__LINE__, &layoutManager, layoutNode.GetPointer(), i))
      {
      return EXIT_FAILURE;
      }
    scene->Clear(false);
    }

  // Note:
  // Qt reports leaks in debug mode (LEAK: 88 WebCoreNode) on exit.
  // This seems to be harmless and will be fixed in future Qt releases.
  // More info: https://bugreports.qt.io/browse/QTBUG-29390

  return EXIT_SUCCESS;
}
