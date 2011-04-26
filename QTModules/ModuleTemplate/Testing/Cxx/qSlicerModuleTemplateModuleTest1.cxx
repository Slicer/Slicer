/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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
#include <QApplication>
#include <QTimer>
#include <QWidget>

// SlicerQt includes
#include <qSlicerAbstractModuleRepresentation.h>

// SlicerModuleTemplate includes
#include "qSlicerModuleTemplateModule.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
int qSlicerModuleTemplateModuleTest1( int argc, char * argv[] )
{
  QApplication app(argc, argv);

  qSlicerModuleTemplateModule moduleTemplateModule;

  if (moduleTemplateModule.helpText().isEmpty())
    {
    std::cerr << "qSlicerModuleTemplateModule::helpText() is empty."
              << std::endl;
    return EXIT_FAILURE;
    }

  if (moduleTemplateModule.acknowledgementText().isEmpty())
    {
    std::cerr << "qSlicerModuleTemplateModule::acknowledgementText() is empty."
              << std::endl;
    return EXIT_FAILURE;
    }

  moduleTemplateModule.initialize(0);
  if (moduleTemplateModule.appLogic() != 0)
    {
    std::cerr << "qSlicerModuleTemplateModule::initialize() failed."
              << std::endl;
    return EXIT_FAILURE;
    }

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  moduleTemplateModule.setMRMLScene(scene);
  if (moduleTemplateModule.mrmlScene() != scene)
    {
    std::cerr << "qSlicerModuleTemplateModule::setMRMLScene() failed."
              << std::endl;
    return EXIT_FAILURE;
    }

  // Instantiate the logic if any
  moduleTemplateModule.logic();

  // Instantiate the widget if any
  QWidget* widget = dynamic_cast<QWidget*>(
    moduleTemplateModule.widgetRepresentation());
  if (widget)
    {
    widget->show();
    }

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
