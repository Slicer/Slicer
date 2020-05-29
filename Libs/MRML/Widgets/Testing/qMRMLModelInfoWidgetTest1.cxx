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

// qMRML includes
#include "qMRMLModelInfoWidget.h"

// MRML includes
#include <vtkMRMLModelNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include "qMRMLWidget.h"

// STD includes

int qMRMLModelInfoWidgetTest1(int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkSmartPointer< vtkMRMLModelNode > modelNode = vtkSmartPointer< vtkMRMLModelNode >::New();

  vtkSmartPointer< vtkPolyData > polyData = vtkSmartPointer< vtkPolyData >::New();
  modelNode->SetAndObservePolyData(polyData);

  qMRMLModelInfoWidget modelInfo;
  modelInfo.setMRMLModelNode(modelNode);
  modelInfo.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}

