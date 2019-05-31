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
#include "qMRMLLayoutManagerTestHelper.cxx"

int qMRMLLayoutManagerTest4(int argc, char * argv[] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  QWidget w;
  w.show();

  qMRMLLayoutManager layoutManager(&w, &w);

  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->SetMRMLApplicationLogic(applicationLogic);

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

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    return safeApplicationQuit(&app);
    }
  else
    {
    return app.exec();
    }
}
