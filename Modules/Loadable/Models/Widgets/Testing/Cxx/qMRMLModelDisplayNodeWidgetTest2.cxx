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
#include <QApplication>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// Models includes
#include <ctkVTKDataSetArrayComboBox.h>

// MRML includes
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include "qMRMLWidget.h"

// STD includes

int qMRMLModelDisplayNodeWidgetTest2( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  if (argc < 2)
    {
    std::cerr << "Usage: qMRMLModelDisplayNodeWidgetTest2 <mrmlScenePath>" << std::endl;
    return EXIT_FAILURE;
    }

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  scene->SetURL(argv[1]);
  scene->Connect();

  vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(
    scene->GetFirstNodeByClass("vtkMRMLModelDisplayNode"));

  if (!modelDisplayNode)
    {
    std::cerr << "Scene: " << argv[1] << " must contain a"
              << " vtkMRMLModelDisplayNode" << std::endl;
    return EXIT_FAILURE;
    }

  //qMRMLModelDisplayNodeWidget modelDisplayNodeWidget;
  //modelDisplayNodeWidget.setMRMLModelDisplayNode(modelDisplayNode);
  //modelDisplayNodeWidget.show();
  ctkVTKDataSetArrayComboBox dataSetModel;
  dataSetModel.setDataSet(modelDisplayNode->GetInputMesh());

  if (argc < 3 || QString(argv[2]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
