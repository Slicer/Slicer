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

// Slicer includes
#include "vtkSlicerConfigure.h"

// Volumes includes
#include "qSlicerDiffusionTensorVolumeDisplayWidget.h"

// MRML includes
#include <vtkMRMLDiffusionTensorVolumeDisplayNode.h>
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include "qMRMLWidget.h"

//-----------------------------------------------------------------------------
int qSlicerDiffusionTensorVolumeDisplayWidgetTest1( int argc, char * argv[] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkSmartPointer<vtkMRMLDiffusionTensorVolumeDisplayNode> displayNode =
    vtkSmartPointer<vtkMRMLDiffusionTensorVolumeDisplayNode>::New();
  scene->AddNode(displayNode);
  vtkSmartPointer<vtkMRMLDiffusionTensorVolumeNode> volumeNode =
    vtkSmartPointer<vtkMRMLDiffusionTensorVolumeNode>::New();
  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  scene->AddNode(volumeNode);

  qSlicerDiffusionTensorVolumeDisplayWidget widget;
  widget.setMRMLScene(scene);
  widget.setMRMLVolumeNode(volumeNode);

  widget.show();
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
