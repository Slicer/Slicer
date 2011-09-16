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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// QT includes
#include <QApplication>
#include <QTimer>

// EMSegment includes
#include "qSlicerEMSegmentAnatomicalTreeWidget.h"
#include "vtkEMSegmentLogic.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSTemplateNode.h>

// Slicer includes
#include <vtkSlicerColorLogic.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

// STD includes
#include <cstdlib>
#include <iostream>

#include "TestingMacros.h"

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

int qSlicerEMSegmentAnatomicalTreeWidgetTest1( int argc, char * argv [] )
{
  if (argc < 2)
    {
    std::cerr<< "Wrong number of arguments." << std::endl;
    return EXIT_FAILURE;
    }
  QApplication app(argc, argv);

  VTK_CREATE(vtkMRMLScene, scene);
  VTK_CREATE(vtkSlicerColorLogic, colorLogic);
  colorLogic->SetMRMLScene(scene);
  colorLogic->AddDefaultColorNodes();

  VTK_CREATE(vtkEMSegmentLogic, logic);
  ctkVerify2(logic->GetMRMLManager(), "Problem with vtkEMSegmentLogic::GetMRMLManager");

  qSlicerEMSegmentAnatomicalTreeWidget anatomicalTreeWidget;
  ctkExerciseMethod(&anatomicalTreeWidget, setMRMLScene, mrmlScene,
                    scene, scene);

  // Set MRMLManager to the widget
  ctkExerciseMethod(&anatomicalTreeWidget, setMRMLManager, mrmlManager,
                    logic->GetMRMLManager(), logic->GetMRMLManager());

  // Set scene to the Logic
  logic->SetMRMLScene(scene);

  // Load Scene
  scene->SetURL(argv[1]);
  int ret = scene->Connect();
  ctkVerify2(ret, "Failed to load scene:" << argv[1]);

  // Get reference of the first vtkMRMLEMSNode
  std::vector<vtkMRMLNode*> emsNodes;
  scene->GetNodesByClass("vtkMRMLEMSTemplateNode", emsNodes);
  ctkVerify2(emsNodes.size() >= 1, "Scene does NOT contain vtkMRMLEMSNode nodes");
  vtkMRMLEMSTemplateNode * emsNode = vtkMRMLEMSTemplateNode::SafeDownCast(emsNodes.at(0));
  ctkVerify2(emsNode, "Problem with vtkMRMLScene::GetNodesByClass");
  ctkExerciseMethod(logic->GetMRMLManager(), SetNodeWithCheck, GetNode,
                    emsNode, emsNode);

  // Set EMSNode to the widget
  anatomicalTreeWidget.updateWidgetFromMRML();
  anatomicalTreeWidget.show();

  if (argc < 3 || QString(argv[2]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
