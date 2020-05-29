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

  This file was originally developed by Luis Ibanez, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerColorsModule.h"
#include "qSlicerColorsModuleWidget.h"

// MRMLLogic includes
#include <vtkMRMLColorLogic.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include "qMRMLWidget.h"

// STD includes

#include "vtkMRMLCoreTestingMacros.h"

int qSlicerColorsModuleWidgetTest1(int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkSmartPointer<vtkMRMLColorLogic> colorLogic = vtkSmartPointer<vtkMRMLColorLogic>::New();
  colorLogic->SetMRMLScene(scene);

  qSlicerColorsModule colorsModule;
  colorsModule.setMRMLScene(scene);
  colorsModule.initialize(nullptr);

  qSlicerColorsModuleWidget* colorsWidget =
    dynamic_cast<qSlicerColorsModuleWidget*>(colorsModule.widgetRepresentation());
  colorsWidget->show();

  std::vector< vtkMRMLNode* > nodes;
  scene->GetNodesByClass("vtkMRMLColorNode", nodes);
  for (std::vector< vtkMRMLNode* >::iterator nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
    {
    colorsWidget->setCurrentColorNode(*nodeIt);
    }

  // colorsWidget->show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(100, qApp, SLOT(quit()));
    }

  return app.exec();
}

