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
#include "qSlicerCoreApplication.h"
#include "qSlicerSimpleMarkupsWidget.h"

// MRML includes
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>

int qSlicerSimpleMarkupsWidgetTest1( int argc, char * argv [] )
{
  qSlicerCoreApplication app(argc, argv);

  qSlicerSimpleMarkupsWidget markupsWidget;
  markupsWidget.show();
  vtkNew<vtkMRMLScene> scene;

  markupsWidget.setMRMLScene(scene.GetPointer());

  if (argc < 3 || QString(argv[2]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
