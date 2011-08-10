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
#include <QTimer>
#include <QWidget>

// SlicerQt includes
#include <qSlicerAbstractModuleRepresentation.h>
#include <qSlicerApplication.h>

// Volumes includes
#include "qSlicerModelsModule.h"
#include "vtkSlicerModelsLogic.h"

// MRML includes
#include <vtkMRMLModelHierarchyNode.h>

// VTK includes
#include <vtkNew.h>

//-----------------------------------------------------------------------------
int qSlicerModelsModuleWidgetTest1( int argc, char * argv[] )
{
  qSlicerApplication app(argc, argv);

  qSlicerModelsModule module;
  module.initialize(0);

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

  dynamic_cast<QWidget*>(module.widgetRepresentation())->show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
