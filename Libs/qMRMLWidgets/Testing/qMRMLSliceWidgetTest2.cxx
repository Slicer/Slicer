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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// QT includes
#include <QApplication>
#include <QTimer>

// qMRML includes
#include "qMRMLSliceControllerWidget.h"
#include "qMRMLSliceWidget.h"
#include "qMRMLNodeObject.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLVolumeArchetypeStorageNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkMultiThreader.h>

// STD includes
#include <cstdlib>
#include <iostream>

vtkMRMLScalarVolumeNode* loadVolume(const char* volume, vtkMRMLScene* scene)
{
  vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> displayNode = vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();
  vtkSmartPointer<vtkMRMLScalarVolumeNode> scalarNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
  vtkSmartPointer<vtkMRMLVolumeArchetypeStorageNode> storageNode = vtkSmartPointer<vtkMRMLVolumeArchetypeStorageNode>::New();

  displayNode->SetAutoWindowLevel(false);
  displayNode->SetInterpolate(false);

  storageNode->SetFileName(volume);
  if (storageNode->SupportedFileType(volume) == 0)
    {
    return 0;
    }
  scalarNode->SetName("foo");
  scalarNode->SetScene(scene);
  displayNode->SetScene(scene);
  //vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
  //displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
  //colorLogic->Delete();
  scene->AddNode(storageNode);
  scene->AddNode(displayNode);
  scalarNode->SetAndObserveStorageNodeID(storageNode->GetID());
  scalarNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  scene->AddNode(scalarNode);
  storageNode->ReadData(scalarNode);

  vtkMRMLColorTableNode* colorNode = vtkMRMLColorTableNode::New();
  colorNode->SetTypeToGrey();
  scene->AddNode(colorNode);
  colorNode->Delete();
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());

  return scalarNode.GetPointer();
}

int qMRMLSliceWidgetTest2(int argc, char * argv [] )
{
  QApplication app(argc, argv);
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  input_image.nrrd " << std::endl;
    return EXIT_FAILURE;
    }

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkMRMLScalarVolumeNode* scalarNode = loadVolume(argv[1], scene);
  if (scalarNode == 0)
    {
    std::cerr << "Not a valid volume: " << argv[1] << std::endl;
    return EXIT_FAILURE;
    }

  QSize viewSize(256, 256);
  qMRMLSliceWidget sliceWidget;
  sliceWidget.setMRMLScene(scene);
  sliceWidget.resize(viewSize.width(), sliceWidget.sliceController()->height() + viewSize.height() );

  vtkMRMLSliceCompositeNode* sliceCompositeNode = sliceWidget.sliceLogic()->GetSliceCompositeNode();
  sliceCompositeNode->SetBackgroundVolumeID(scalarNode->GetID());
  sliceWidget.show();

  qMRMLNodeObject nodeObject(scalarNode->GetDisplayNode(), &sliceWidget);
  nodeObject.setProcessEvents(false);
  nodeObject.setMessage("vtkMRMLDisplayNode");
  for (int i = 0; i < 30; ++i)
    {
    nodeObject.modify();
    }
  nodeObject.setProcessEvents(true);
  nodeObject.setMessage("vtkMRMLDisplayNode + render");
  for (int i = 0; i < 30; ++i)
    {
    nodeObject.modify();
    }
/*
  QTimer modifyTimer;
  modifyTimer.setInterval(0);
  QObject::connect(&modifyTimer, SIGNAL(timeout()),
                   &nodeObject, SLOT(modify()));
  modifyTimer.start();
*/
  if (argc < 3 || QString(argv[2]) != "-I" )
    {
    QTimer::singleShot(1000, &app, SLOT(quit()));
    }
  return app.exec();
}
