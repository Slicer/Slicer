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
#include <QSignalSpy>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRML includes
#include "qMRMLNodeComboBox.h"

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

int qMRMLNodeComboBoxTest5( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  qMRMLNodeComboBox nodeSelector;
  nodeSelector.setNodeTypes(QStringList("vtkMRMLCameraNode"));
  nodeSelector.setNoneEnabled(true);

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLCameraNode> camNode;
  scene->AddNode(camNode.GetPointer());

  nodeSelector.setMRMLScene(scene.GetPointer());

  if (nodeSelector.currentNode() != nullptr)
    {
    std::cerr << "qMRMLNodeComboBox::setMRMLScene() failed: " << std::endl;
    return EXIT_FAILURE;
    }

  QSignalSpy spy(&nodeSelector, SIGNAL(currentNodeChanged(bool)));
  nodeSelector.setCurrentNode(camNode.GetPointer());
  if (spy.count() != 1)
    {
    std::cerr << "qMRMLNodeComboBox::setCurrentNode() failed: "
              << spy.count() << std::endl;
    return EXIT_FAILURE;
    }
  spy.clear();
  nodeSelector.setCurrentNode(nullptr);
  if (spy.count() != 1)
    {
    std::cerr << "qMRMLNodeComboBox::setCurrentNode() failed: "
              << spy.count() << std::endl;
    return EXIT_FAILURE;
    }
  spy.clear();
  nodeSelector.setCurrentNode(camNode.GetPointer());
  if (spy.count() != 1)
    {
    std::cerr << "qMRMLNodeComboBox::setCurrentNode() failed: "
              << spy.count() << std::endl;
    return EXIT_FAILURE;
    }

  nodeSelector.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
