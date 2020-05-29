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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRML includes
#include "qMRMLSliceWidget.h"
#include "qMRMLNodeObject.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkMultiThreader.h>
#include <vtkNew.h>
#include "qMRMLWidget.h"

int qMRMLSliceWidgetTest1(int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();
  vtkMultiThreader::SetGlobalMaximumNumberOfThreads(1);
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.mrml " << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->SetMRMLApplicationLogic(applicationLogic);
  applicationLogic->SetMRMLScene(scene.GetPointer());
  scene->SetURL(argv[1]);
  scene->Connect();
  if (scene->GetNumberOfNodes() == 0)
    {
    std::cerr << "Can't load scene:" << argv[1] << " error: " <<scene->GetErrorMessage() << std::endl;
    return EXIT_FAILURE;
    }
  vtkMRMLSliceNode* redSliceNode = nullptr;
  // search for a red slice node
  std::vector<vtkMRMLNode*> sliceNodes;
  scene->GetNodesByClass("vtkMRMLSliceNode", sliceNodes);
  for (unsigned int i = 0; i < sliceNodes.size(); ++i)
    {
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(sliceNodes[i]);
    if (!strcmp(sliceNode->GetLayoutName(), "Red") )
      {
      redSliceNode = sliceNode;
      break;
      }
    }
  if (!redSliceNode)
    {
    std::cerr << "Scene must contain a valid vtkMRMLSliceNode:" << redSliceNode << std::endl;
    return EXIT_FAILURE;
    }
  vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLScalarVolumeNode");
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);
  if (!volumeNode)
    {
    std::cerr << "Scene must contain a valid vtkMRMLVolumeNode:" << node << std::endl;
    return EXIT_FAILURE;
    }
  if (!volumeNode->GetDisplayNode()->GetColorNode())
    {
    // add a custom color node (grey)
    vtkMRMLColorTableNode* colorNode = vtkMRMLColorTableNode::New();
    colorNode->SetTypeToGrey();
    scene->AddNode(colorNode);
    colorNode->Delete();
    volumeNode->GetDisplayNode()->SetAndObserveColorNodeID(colorNode->GetID());
    }

  // "Red" slice by default
  qMRMLSliceWidget sliceWidget;
  sliceWidget.setMRMLScene(scene.GetPointer());
  sliceWidget.setMRMLSliceNode(redSliceNode);
  sliceWidget.show();

  qMRMLNodeObject nodeObject(volumeNode, &sliceWidget);
  QTimer modifyTimer;
  modifyTimer.setInterval(2000);
  QObject::connect(&modifyTimer, SIGNAL(timeout()),
                   &nodeObject, SLOT(modify()));
  //modifyTimer.start();
  if (argc < 3 || QString(argv[2]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}

