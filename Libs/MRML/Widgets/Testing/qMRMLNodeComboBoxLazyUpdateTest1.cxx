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
#include <QDebug>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRML includes
#include "qMRMLColorTableComboBox.h"
#include "qMRMLNodeComboBox.h"
#include "qMRMLSceneModel.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes

int qMRMLNodeComboBoxLazyUpdateTest1( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  qMRMLNodeComboBox nodeSelector;
  qMRMLColorTableComboBox treeNodeSelector;

  nodeSelector.setNodeTypes(QStringList("vtkMRMLColorTableNode"));
  nodeSelector.setShowHidden(true);
  nodeSelector.setNoneEnabled(true);

  qobject_cast<qMRMLSceneModel*>(nodeSelector.sortFilterProxyModel()->sourceModel())
    ->setLazyUpdate(true);
  qobject_cast<qMRMLSceneModel*>(treeNodeSelector.sortFilterProxyModel()->sourceModel())
    ->setLazyUpdate(true);

  vtkNew<vtkMRMLScene> scene;
  nodeSelector.setMRMLScene(scene.GetPointer());
  treeNodeSelector.setMRMLScene(scene.GetPointer());

  scene->StartState(vtkMRMLScene::ImportState);
  vtkNew<vtkMRMLColorTableNode> node;
  scene->AddNode(node.GetPointer());

  if (nodeSelector.nodeCount() != 0 ||
      treeNodeSelector.nodeCount() != 0 )
    {
    std::cerr << "qMRMLSceneModel::LazyUpdate failed when adding a node"
              << std::endl;
    return EXIT_FAILURE;
    }

  scene->EndState(vtkMRMLScene::ImportState);

  if (nodeSelector.nodeCount() != 1 ||
      treeNodeSelector.nodeCount() != 1 )
    {
    std::cerr << "qMRMLSceneModel::LazyUpdate failed when updating the scene"
              << std::endl;
    return EXIT_FAILURE;
    }

  qMRMLNodeComboBox nodeSelector2;
  nodeSelector2.setNodeTypes(QStringList("vtkMRMLColorTableNode"));
  nodeSelector2.setShowHidden(true);
  nodeSelector2.setNoneEnabled(true);

  qMRMLColorTableComboBox treeNodeSelector2;

  qobject_cast<qMRMLSceneModel*>(nodeSelector2.sortFilterProxyModel()->sourceModel())
    ->setLazyUpdate(true);
  qobject_cast<qMRMLSceneModel*>(treeNodeSelector2.sortFilterProxyModel()->sourceModel())
    ->setLazyUpdate(true);

  nodeSelector2.setMRMLScene(scene.GetPointer());
  treeNodeSelector2.setMRMLScene(scene.GetPointer());

  if (nodeSelector2.nodeCount() != 1 ||
      treeNodeSelector2.nodeCount() != 1 )
    {
    std::cerr << "qMRMLSceneModel::LazyUpdate failed when updating the scene"
              << std::endl;
    return EXIT_FAILURE;
    }

  nodeSelector.show();
  nodeSelector2.show();

  treeNodeSelector.show();
  treeNodeSelector2.show();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
