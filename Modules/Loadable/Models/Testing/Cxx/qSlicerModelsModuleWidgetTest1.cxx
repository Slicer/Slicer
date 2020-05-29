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
#include <QTimer>
#include <QWidget>

// Slicer includes
#include "vtkSlicerConfigure.h"

// Slicer includes
#include <qSlicerAbstractModuleRepresentation.h>
#include <qSlicerApplication.h>

// Volumes includes
#include "qSlicerModelsModule.h"
#include "vtkSlicerModelsLogic.h"

// MRML includes
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

//-----------------------------------------------------------------------------
int qSlicerModelsModuleWidgetTest1( int argc, char * argv[] )
{
  qMRMLWidget::preInitializeApplication();
  qSlicerApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  if (argc < 2)
    {
    std::cerr << "Usage: qSlicerModelsModuleWidgetTest1 sceneFilePath [-I]"
              << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerModelsModule module;
  module.initialize(nullptr);

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkSlicerModelsLogic> modelsLogic;
  modelsLogic->SetMRMLScene(scene.GetPointer());

  vtkMRMLModelNode* modelNode = modelsLogic->AddModel(argv[1]);
  if (!modelNode)
    {
    std::cerr << "Bad model file:" << argv[1] << std::endl;
    return EXIT_FAILURE;
    }

  module.setMRMLScene(scene.GetPointer());

  vtkNew<vtkMRMLModelHierarchyNode> hierarchyNode;
  scene->AddNode(hierarchyNode.GetPointer());

  vtkNew<vtkMRMLModelHierarchyNode> hierarchyNode2;
  scene->AddNode(hierarchyNode2.GetPointer());

  vtkNew<vtkMRMLModelHierarchyNode> hierarchyNode3;
  scene->AddNode(hierarchyNode3.GetPointer());

  hierarchyNode3->SetParentNodeID(hierarchyNode2->GetID());

  vtkMRMLModelNode* modelNode2 = modelsLogic->AddModel(argv[1]);
  hierarchyNode3->SetAssociatedNodeID(modelNode2->GetID());
  dynamic_cast<QWidget*>(module.widgetRepresentation())->show();

  if (argc < 3 || QString(argv[2]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
