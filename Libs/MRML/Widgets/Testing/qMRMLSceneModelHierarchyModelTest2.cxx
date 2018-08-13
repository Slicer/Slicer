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

// QT includes
#include <QApplication>
#include <QTimer>
#include <QTreeView>

// Slicer includes
#include "vtkSlicerConfigure.h"

// CTK includes
#include <ctkModelTester.h>

// qMRML includes
#include "qMRMLSceneFactoryWidget.h"
#include "qMRMLSceneModelHierarchyModel.h"
#include "qMRMLTreeView.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#ifdef Slicer_VTK_USE_QVTKOPENGLWIDGET
#include <QSurfaceFormat>
#include <QVTKOpenGLWidget.h>
#endif

// STD includes

int qMRMLSceneModelHierarchyModelTest2(int argc, char * argv [])
{
#ifdef Slicer_VTK_USE_QVTKOPENGLWIDGET
  // Set default surface format for QVTKOpenGLWidget
  QSurfaceFormat format = QVTKOpenGLWidget::defaultFormat();
  format.setSamples(0);
  QSurfaceFormat::setDefaultFormat(format);
#endif

  QApplication app(argc, argv);

  if (argc < 2)
    {
    std::cout << "Usage: qMRMLSceneModelHierarchyModelTest2 sceneFilePath [-I]"
              << std::endl;
    return EXIT_FAILURE;
    }
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene.GetPointer());
  scene->SetURL( argv[1] );
  scene->Connect();

  qMRMLSceneModelHierarchyModel model;
  model.setMRMLScene(scene.GetPointer());

  QTreeView view;
  view.setWindowTitle("QTreeView");
  view.setDragDropMode(QAbstractItemView::InternalMove);
  view.setModel(&model);
  view.show();

  qMRMLTreeView mrmlView;
  mrmlView.setWindowTitle("qMRMLTreeView");
  mrmlView.setDragDropMode(QAbstractItemView::InternalMove);
  mrmlView.setSceneModelType("ModelHierarchy");
  mrmlView.setNodeTypes(QStringList() << "vtkMRMLModelNode" << "vtkMRMLModelHierarchyNode");
  mrmlView.sortFilterProxyModel()->setShowHiddenForTypes(
    QStringList() << "vtkMRMLModelHierarchyNode");
  mrmlView.setMRMLScene(scene.GetPointer());
  mrmlView.show();

  qMRMLTreeView mrmlView2;
  mrmlView2.setWindowTitle("qMRMLTreeView2");
  mrmlView2.setDragDropMode(QAbstractItemView::InternalMove);
  mrmlView2.setSceneModelType("ModelHierarchy");
  mrmlView2.sortFilterProxyModel()->setHideChildNodeTypes(
    QStringList() << "vtkMRMLFiberBundleNode" << "vtkMRMLAnnotationNode");
  mrmlView2.sortFilterProxyModel()->setShowHiddenForTypes(
    QStringList() << "vtkMRMLModelHierarchyNode");
  mrmlView2.setMRMLScene(scene.GetPointer());
  mrmlView2.show();

  if (argc < 3 || QString(argv[2]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

