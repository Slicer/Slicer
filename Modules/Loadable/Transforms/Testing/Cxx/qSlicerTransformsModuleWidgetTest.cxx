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

// CTK includes
#include "ctkTest.h"

// MRML includes
#include <qMRMLWidget.h>
#include "qSlicerTransformsModule.h"
#include "qSlicerTransformsModuleWidget.h"
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>

// SubjectHierarchy includes
#include <qSlicerSubjectHierarchyPluginHandler.h>
#include <qSlicerSubjectHierarchyPluginLogic.h>
#include <qSlicerSubjectHierarchyTransformsPlugin.h>
#include <vtkSlicerSubjectHierarchyModuleLogic.h>

// Slicer includes
#include <qSlicerApplication.h>

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>

// ----------------------------------------------------------------------------
class qSlicerTransformsModuleWidgetTester : public QObject
{
  Q_OBJECT

public:
  QSharedPointer<qSlicerTransformsModule> transformsModule;
  QSharedPointer<qSlicerSubjectHierarchyPluginLogic> pluginLogic;
  vtkSmartPointer<vtkSlicerSubjectHierarchyModuleLogic> shModuleLogic;
  vtkSmartPointer<vtkMRMLScene> scene;

  bool interactive{ false };

private slots:

  void initTestCase();
  void cleanupTestCase();

  void testIdentity();
  void testInvert();
};

// ----------------------------------------------------------------------------
void qSlicerTransformsModuleWidgetTester::initTestCase()
{
  // Transforms module must be initialized once and only once and we do it here
  // (multiple initializations would result in re-registering plugins,
  // which would lead to warnings that would make the test fail).

  scene = vtkSmartPointer<vtkMRMLScene>::New();

  // Set up Subject Hierarchy logic (needed for the subject hierarchy tree view)
  shModuleLogic = vtkSmartPointer<vtkSlicerSubjectHierarchyModuleLogic>::New();
  shModuleLogic->SetMRMLScene(scene);
  pluginLogic = QSharedPointer<qSlicerSubjectHierarchyPluginLogic>(new qSlicerSubjectHierarchyPluginLogic());
  pluginLogic->setMRMLScene(scene);
  qSlicerSubjectHierarchyPluginHandler::instance()->setPluginLogic(pluginLogic.get());
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);

  transformsModule = QSharedPointer<qSlicerTransformsModule>(new qSlicerTransformsModule);
  // register qSlicerSubjectHierarchyTransformsPlugin
  transformsModule->initialize(qSlicerApplication::application()->applicationLogic());
  transformsModule->setMRMLScene(scene);
  transformsModule->logic();
}

// ----------------------------------------------------------------------------
void qSlicerTransformsModuleWidgetTester::cleanupTestCase()
{
  transformsModule.clear();
  pluginLogic.clear();
  shModuleLogic = nullptr;
  scene = nullptr;
}

// ----------------------------------------------------------------------------
void qSlicerTransformsModuleWidgetTester::testIdentity()
{
  qSlicerTransformsModuleWidget* transformsWidget = dynamic_cast<qSlicerTransformsModuleWidget*>(transformsModule->widgetRepresentation());

  scene->Clear();
  vtkNew<vtkMRMLTransformNode> transformNode;
  scene->AddNode(transformNode);

  transformsWidget->setEditedNode(transformNode);

  vtkNew<vtkMatrix4x4> matrix;
  transformNode->GetMatrixTransformToParent(matrix);
  matrix->SetElement(0, 0, 10.);
  matrix->SetElement(1, 0, 2.);
  transformNode->SetMatrixTransformToParent(matrix);
  transformsWidget->identity();
  transformNode->GetMatrixTransformToParent(matrix);

  if (interactive)
  {
    transformsWidget->show();
    qApp->exec();
  }

  QCOMPARE(matrix->GetElement(0, 0), 1.);
  QCOMPARE(matrix->GetElement(1, 0), 0.);
}

// ----------------------------------------------------------------------------
void qSlicerTransformsModuleWidgetTester::testInvert()
{
  qSlicerTransformsModuleWidget* transformsWidget = dynamic_cast<qSlicerTransformsModuleWidget*>(transformsModule->widgetRepresentation());

  scene->Clear();
  vtkNew<vtkMRMLTransformNode> transformNode;
  scene->AddNode(transformNode);

  transformsWidget->setEditedNode(transformNode);

  vtkNew<vtkMatrix4x4> matrix;
  transformNode->GetMatrixTransformToParent(matrix);
  matrix->SetElement(0, 0, 10.);
  matrix->SetElement(1, 0, 2.);
  transformNode->SetMatrixTransformToParent(matrix);
  transformsWidget->invert();
  transformNode->GetMatrixTransformToParent(matrix);

  if (interactive)
  {
    transformsWidget->show();
    qApp->exec();
  }

  QCOMPARE(matrix->GetElement(0, 0), 0.1);
  QCOMPARE(matrix->GetElement(1, 0), -0.2);
}

// ----------------------------------------------------------------------------
// CTK_TEST_MAIN(qSlicerTransformsModuleWidgetTest)
int qSlicerTransformsModuleWidgetTest(int argc, char* argv[])
{
  qMRMLWidget::preInitializeApplication();
  const qSlicerApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  qSlicerTransformsModuleWidgetTester tc;
  tc.interactive = false; // set it to true for interactive testing

  return QTest::qExec(&tc, argc, argv);
}

#include "moc_qSlicerTransformsModuleWidgetTest.cxx"
