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
#include "qMRMLSceneModel.h"

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes
#include <iostream>

int qMRMLNodeComboBoxTest5(int argc, char* argv[])
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
    std::cerr << "qMRMLNodeComboBox::setCurrentNode() failed: " << spy.count() << std::endl;
    return EXIT_FAILURE;
  }
  spy.clear();
  nodeSelector.setCurrentNode(nullptr);
  if (spy.count() != 1)
  {
    std::cerr << "qMRMLNodeComboBox::setCurrentNode() failed: " << spy.count() << std::endl;
    return EXIT_FAILURE;
  }
  spy.clear();
  nodeSelector.setCurrentNode(camNode.GetPointer());
  if (spy.count() != 1)
  {
    std::cerr << "qMRMLNodeComboBox::setCurrentNode() failed: " << spy.count() << std::endl;
    return EXIT_FAILURE;
  }

  // vtkMRMLNode* must be registered as a Qt metatype for QSignalSpy to
  // capture signals with vtkMRMLNode* arguments.
  qRegisterMetaType<vtkMRMLNode*>("vtkMRMLNode*");

  // --- Test: nodeAdded / nodeAddedByUser / nodeAddedByUserAction signal counts ---
  //
  // A programmatic addNode() call should emit exactly one nodeAdded and one
  // nodeAddedByUser signal, but no nodeAddedByUserAction signal.
  //
  // A GUI-triggered add (clicking the "Create new..." extra item) should emit
  // one nodeAdded, one nodeAddedByUser, AND one nodeAddedByUserAction signal.

  vtkNew<vtkMRMLScene> addNodeScene;
  qMRMLNodeComboBox addNodeSelector;
  addNodeSelector.setNodeTypes(QStringList("vtkMRMLScalarVolumeNode"));
  addNodeSelector.setAddEnabled(true);
  addNodeSelector.setMRMLScene(addNodeScene.GetPointer());

  // Programmatic add: addNode() should emit nodeAdded x1, nodeAddedByUser x1,
  // nodeAddedByUserAction x0.
  {
    QSignalSpy nodeAddedSpy(&addNodeSelector, SIGNAL(nodeAdded(vtkMRMLNode*)));
    QSignalSpy nodeAddedByUserSpy(&addNodeSelector, SIGNAL(nodeAddedByUser(vtkMRMLNode*)));
    QSignalSpy nodeAddedByUserActionSpy(&addNodeSelector, SIGNAL(nodeAddedByUserAction(vtkMRMLNode*)));

    vtkMRMLNode* newNode = addNodeSelector.addNode();
    if (!newNode)
    {
      std::cerr << __LINE__ << " - addNode() returned null." << std::endl;
      return EXIT_FAILURE;
    }
    if (nodeAddedSpy.count() != 1)
    {
      std::cerr << __LINE__ << " - Programmatic addNode() should emit nodeAdded exactly once,"
                << " got " << nodeAddedSpy.count() << std::endl;
      return EXIT_FAILURE;
    }
    if (nodeAddedByUserSpy.count() != 1)
    {
      std::cerr << __LINE__ << " - Programmatic addNode() should emit nodeAddedByUser exactly once,"
                << " got " << nodeAddedByUserSpy.count() << std::endl;
      return EXIT_FAILURE;
    }
    if (nodeAddedByUserActionSpy.count() != 0)
    {
      std::cerr << __LINE__ << " - Programmatic addNode() should not emit nodeAddedByUserAction,"
                << " got " << nodeAddedByUserActionSpy.count() << std::endl;
      return EXIT_FAILURE;
    }
    std::cout << "Programmatic addNode() signal test passed." << std::endl;
  }

  // GUI add: activateExtraItem with a CREATE index (simulating a click on the
  // "Create new..." dropdown item) should emit all three signals exactly once.
  // The combobox rootModelIndex is set to model->index(0,0), the proxy-mapped
  // mrmlSceneItem; extra items (CREATE/...) are children of that index.
  {
    QSignalSpy nodeAddedSpy(&addNodeSelector, SIGNAL(nodeAdded(vtkMRMLNode*)));
    QSignalSpy nodeAddedByUserSpy(&addNodeSelector, SIGNAL(nodeAddedByUser(vtkMRMLNode*)));
    QSignalSpy nodeAddedByUserActionSpy(&addNodeSelector, SIGNAL(nodeAddedByUserAction(vtkMRMLNode*)));

    QAbstractItemModel* model = addNodeSelector.model();
    QModelIndex sceneItemIndex = model->index(0, 0, QModelIndex());
    QModelIndex createIndex;
    for (int i = 0; i < model->rowCount(sceneItemIndex); ++i)
    {
      QModelIndex idx = model->index(i, 0, sceneItemIndex);
      if (model->data(idx, qMRMLSceneModel::ExtraItemsRole).toString().startsWith("CREATE"))
      {
        createIndex = idx;
        break;
      }
    }
    if (!createIndex.isValid())
    {
      std::cerr << __LINE__ << " - Could not find CREATE extra item in combobox model." << std::endl;
      return EXIT_FAILURE;
    }

    // activateExtraItem is a protected slot; QMetaObject::invokeMethod bypasses
    // C++ access control, invoking it exactly as a user click on the item would.
    bool invoked = QMetaObject::invokeMethod(&addNodeSelector, "activateExtraItem", Qt::DirectConnection, Q_ARG(QModelIndex, createIndex));
    if (!invoked)
    {
      std::cerr << __LINE__ << " - Failed to invoke activateExtraItem." << std::endl;
      return EXIT_FAILURE;
    }
    if (nodeAddedSpy.count() != 1)
    {
      std::cerr << __LINE__ << " - GUI addNode should emit nodeAdded exactly once,"
                << " got " << nodeAddedSpy.count() << std::endl;
      return EXIT_FAILURE;
    }
    if (nodeAddedByUserSpy.count() != 1)
    {
      std::cerr << __LINE__ << " - GUI addNode should emit nodeAddedByUser exactly once,"
                << " got " << nodeAddedByUserSpy.count() << std::endl;
      return EXIT_FAILURE;
    }
    if (nodeAddedByUserActionSpy.count() != 1)
    {
      std::cerr << __LINE__ << " - GUI addNode should emit nodeAddedByUserAction exactly once,"
                << " got " << nodeAddedByUserActionSpy.count() << std::endl;
      return EXIT_FAILURE;
    }
    std::cout << "GUI addNode (activateExtraItem) signal test passed." << std::endl;
  }

  nodeSelector.show();

  if (argc < 2 || QString(argv[1]) != "-I")
  {
    QTimer::singleShot(200, &app, SLOT(quit()));
  }

  return app.exec();
}
