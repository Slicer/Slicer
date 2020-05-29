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
#include <QVBoxLayout>

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRML includes
#include "qMRMLSliceWidget.h"
#include "qMRMLVolumeThresholdWidget.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLVolumeNode.h>

// MRMLLogic includes
#include <vtkMRMLColorLogic.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes

int qMRMLVolumeThresholdWidgetTest2(int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.mrml " << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene.GetPointer());
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->SetMRMLApplicationLogic(applicationLogic);

  // Add default color nodes
  vtkNew<vtkMRMLColorLogic> colorLogic;
  colorLogic->SetMRMLScene(scene.GetPointer());

  scene->SetURL(argv[1]);
  scene->Connect();
  if (scene->GetNumberOfNodes() == 0)
    {
    std::cerr << "Can't load scene:" << argv[1] << " error: " <<scene->GetErrorMessage() << std::endl;
    return EXIT_FAILURE;
    }
  vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLScalarVolumeNode");
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);
  if (!volumeNode)
    {
    std::cerr << "Scene must contain a valid vtkMRMLVolumeNode:" << node << std::endl;
    return EXIT_FAILURE;
    }
  vtkMRMLSliceNode* redSliceNode = nullptr;
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

  QWidget topLevel;
  qMRMLVolumeThresholdWidget volumeThreshold;
  qMRMLSliceWidget sliceWidget;
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(&volumeThreshold);
  layout->addWidget(&sliceWidget);
  topLevel.setLayout(layout);

  volumeThreshold.setMRMLVolumeNode(volumeNode);
  sliceWidget.setMRMLScene(scene.GetPointer());
  sliceWidget.setMRMLSliceNode(redSliceNode);
  topLevel.show();

  if (argc < 3 || QString(argv[2]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

