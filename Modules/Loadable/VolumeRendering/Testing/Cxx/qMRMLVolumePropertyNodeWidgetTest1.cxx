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

// qMRML includes
#include "qMRMLVolumePropertyNodeWidget.h"

// MRML includes
#include <vtkMRMLVolumePropertyNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include "qMRMLWidget.h"

// this test only works on VTKv6 and later

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingContextOpenGL2);

// STD includes

int qMRMLVolumePropertyNodeWidgetTest1(int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkSmartPointer<vtkMRMLVolumePropertyNode> volumePropertyNode =
    vtkSmartPointer<vtkMRMLVolumePropertyNode>::New();

  qMRMLVolumePropertyNodeWidget widget;
  widget.setMRMLVolumePropertyNode(volumePropertyNode);

  widget.show();

  if (argc < 2 || QString(argv[1]) != "-I" )
  {
    QTimer::singleShot(200, &app, SLOT(quit()));
  }
  return app.exec();
}
