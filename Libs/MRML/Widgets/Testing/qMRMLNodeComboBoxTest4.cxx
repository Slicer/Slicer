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

// qMRML includes
#include "qMRMLNodeComboBox.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"


int qMRMLNodeComboBoxTest4( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  qMRMLNodeComboBox nodeSelector;
  nodeSelector.setNodeTypes(QStringList() << "vtkMRMLScalarVolumeNode" << "vtkMRMLLabelMapVolumeNode");
  nodeSelector.setBaseName("SomeBaseName");

  vtkNew<vtkMRMLScene> scene;
  nodeSelector.setMRMLScene(scene.GetPointer());

  vtkMRMLNode* node = nodeSelector.addNode();
  if (nodeSelector.nodeCount() != 1)
    {
    std::cerr << __LINE__ << "qMRMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }

  node->SetName("foo");

  node = nodeSelector.addNode("vtkMRMLScalarVolumeNode");
  if (nodeSelector.nodeCount() != 2)
    {
    std::cerr << __LINE__ << "qMRMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(node->GetName(), "SomeBaseName_1")!=0)
    {
    std::cerr << __LINE__ << "qMRMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }

  node = nodeSelector.addNode("vtkMRMLLabelMapVolumeNode");
  if (nodeSelector.nodeCount() != 3)
    {
    std::cerr << __LINE__ << "qMRMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(node->GetName(), "SomeBaseName_2")!=0)
    {
    std::cerr << __LINE__ << "qMRMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }

  // Test that we cannot add a node type that is not among the list of allowed node types
  node = nodeSelector.addNode("vtkMRMLModelNode");
  if (nodeSelector.nodeCount() != 3)
    {
    std::cerr << __LINE__ << "qMRMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }

  // Check if base name of a particular class can be changed
  nodeSelector.setBaseName("DifferentBaseName", "vtkMRMLLabelMapVolumeNode");
  node = nodeSelector.addNode("vtkMRMLLabelMapVolumeNode");
  if (strcmp(node->GetName(), "DifferentBaseName")!=0)
    {
    std::cerr << __LINE__ << "qMRMLNodeComboBox::addNode is broken" << std::endl;
    return EXIT_FAILURE;
    }

  nodeSelector.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
