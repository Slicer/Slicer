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
#include <QApplication>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// MRMLWidgets includes
#include <qMRMLTreeView.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelDisplayNode.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes

int qMRMLModelTreeViewTest1( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkNew<vtkMRMLModelNode> modelNode;
  vtkNew<vtkMRMLModelDisplayNode> displayModelNode;

  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(modelNode.GetPointer());
  scene->AddNode(displayModelNode.GetPointer());

  modelNode->SetAndObserveDisplayNodeID(displayModelNode->GetID());

  qMRMLTreeView modelView;
  modelView.setSceneModelType("ModelHierarchy");
  modelView.setMRMLScene(scene.GetPointer());

  modelView.show();
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(100, &app, SLOT(quit()));
    }

  return app.exec();
}
