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
#include "qMRMLThreeDWidget.h"
#include "qMRMLThreeDViewControllerWidget.h"

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

int qMRMLThreeDWidgetTest1(int argc, char* argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkNew<vtkMRMLScene> scene;

  // vtkMRMLAbstractDisplayableManager requires selection and interaction nodes
  scene->AddNewNodeByClass("vtkMRMLSelectionNode");
  scene->AddNewNodeByClass("vtkMRMLInteractionNode");

  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLViewNode"));
  // This is required for the view to be connected to the camera displayable manager
  viewNode->SetLayoutName("1");
  // This label is displayed in the widget title bar
  viewNode->SetLayoutLabel("3D");

  qMRMLThreeDWidget widget;
  widget.setMRMLScene(scene);
  widget.setMRMLViewNode(viewNode);
  widget.resize(600, 600);
  widget.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
  {
    QTimer::singleShot(200, &app, SLOT(quit()));
  }
  return app.exec();
}
