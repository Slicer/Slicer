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
#include "qMRMLVolumeThresholdWidget.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes

int qMRMLVolumeThresholdWidgetTest1(int argc, char * argv [] )
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

  qMRMLVolumeThresholdWidget volumeThreshold;
  volumeThreshold.setMRMLVolumeNode(volumeNode);

  volumeThreshold.show();

  if (argc < 3 || QString(argv[2]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}

