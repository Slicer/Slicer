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
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRML includes
#include "qMRMLNavigationView.h"
#include "qMRMLThreeDView.h"

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes

int qMRMLNavigationViewTest1(int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  qMRMLNavigationView navigationView;
  navigationView.setWindowTitle("Navigation view");

  qMRMLThreeDView threeDView;
  threeDView.setWindowTitle("ThreeDView");
  navigationView.setRendererToListen(threeDView.renderer());

  vtkNew<vtkMRMLScene> scene;
  navigationView.setMRMLScene(scene.GetPointer());
  threeDView.setMRMLScene(scene.GetPointer());

  // vtkMRMLAbstractDisplayableManager requires selection and interaction nodes
  vtkNew<vtkMRMLSelectionNode> selectionNode;
  scene->AddNode(selectionNode.GetPointer());
  vtkNew<vtkMRMLInteractionNode> interactionNode;
  scene->AddNode(interactionNode.GetPointer());

  vtkNew<vtkMRMLViewNode> viewNode;
  viewNode->SetBoxVisible(true);
  scene->AddNode(viewNode.GetPointer());

  threeDView.setMRMLViewNode(viewNode.GetPointer());
  navigationView.setMRMLViewNode(viewNode.GetPointer());

  navigationView.show();
  threeDView.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }


  return app.exec();
}
