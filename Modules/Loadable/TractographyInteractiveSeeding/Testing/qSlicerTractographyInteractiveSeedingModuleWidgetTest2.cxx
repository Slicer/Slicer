/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QTimer>

// Slicer includes
#include <qSlicerApplication.h>

// Volumes includes
#include "qSlicerTractographyInteractiveSeedingModule.h"
#include "qSlicerTractographyInteractiveSeedingModuleWidget.h"
#include <vtkSlicerVolumesLogic.h>

// MRML includes

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
int qSlicerTractographyInteractiveSeedingModuleWidgetTest2( int argc, char * argv[] )
{
  qSlicerApplication app(argc, argv);

  qSlicerTractographyInteractiveSeedingModule module;
  module.setMRMLScene(app.mrmlScene());
  module.initialize(0);

  qSlicerTractographyInteractiveSeedingModuleWidget* moduleWidget =
    dynamic_cast<qSlicerTractographyInteractiveSeedingModuleWidget*>(
      module.widgetRepresentation());

  vtkMRMLVolumeNode* volumeNode = 0;
  bool fileNameInTheCommandLine = false;
  if (argc > 1 && QString(argv[1]) != "-I")
    {
    fileNameInTheCommandLine = true;
    vtkSmartPointer<vtkSlicerVolumesLogic> logic = vtkSmartPointer<vtkSlicerVolumesLogic>::New();
    logic->SetMRMLScene(app.mrmlScene());
    volumeNode = logic->AddArchetypeVolume(argv[1], "dtiVolume");
    }

  moduleWidget->setDiffusionTensorVolumeNode(volumeNode);

  moduleWidget->show();

  if (argc < (fileNameInTheCommandLine ? 3 : 2) ||
      QString(argv[fileNameInTheCommandLine ? 2 : 1]) != "-I")
    {
    QTimer::singleShot(100, qApp, SLOT(quit()));
    }

  return app.exec();
}
