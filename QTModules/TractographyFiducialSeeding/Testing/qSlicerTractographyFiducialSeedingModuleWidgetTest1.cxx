/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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
#include <qSlicerApplication.h>

// Volumes includes
#include "qSlicerTractographyFiducialSeedingModule.h"
#include "qSlicerTractographyFiducialSeedingModuleWidget.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
int qSlicerTractographyFiducialSeedingModuleWidgetTest1( int argc, char * argv[] )
{
  qSlicerApplication app(argc, argv);

  qSlicerTractographyFiducialSeedingModule module;
  module.setMRMLScene(app.mrmlScene());
  module.initialize(0);

  qSlicerTractographyFiducialSeedingModuleWidget* moduleWidget =
    dynamic_cast<qSlicerTractographyFiducialSeedingModuleWidget*>(
      module.widgetRepresentation());

  moduleWidget->show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(100, qApp, SLOT(quit()));
    }

  return app.exec();
}
