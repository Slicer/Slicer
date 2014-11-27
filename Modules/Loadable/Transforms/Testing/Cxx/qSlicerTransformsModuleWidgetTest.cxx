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
#include "qSlicerTransformsModule.h"
#include "qSlicerTransformsModuleWidget.h"
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>

// ----------------------------------------------------------------------------
class qSlicerTransformsModuleWidgetTester: public QObject
{
  Q_OBJECT

private slots:

  void testIdentity();
  void testInvert();
};

// ----------------------------------------------------------------------------
void qSlicerTransformsModuleWidgetTester::testIdentity()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLTransformNode> transformNode;
  scene->AddNode(transformNode.GetPointer());

  qSlicerTransformsModule transformsModule;
  transformsModule.setMRMLScene(scene.GetPointer());
  transformsModule.logic();
  qSlicerTransformsModuleWidget* transformsWidget =
    dynamic_cast<qSlicerTransformsModuleWidget*>(transformsModule.widgetRepresentation());

  vtkNew<vtkMatrix4x4> matrix;
  transformNode->GetMatrixTransformToParent(matrix.GetPointer());
  matrix->SetElement(0,0, 10.);
  matrix->SetElement(1,0, 2.);
  transformNode->SetMatrixTransformToParent(matrix.GetPointer());
  transformsWidget->identity();
  transformNode->GetMatrixTransformToParent(matrix.GetPointer());
  QCOMPARE(matrix->GetElement(0,0), 1.);
  QCOMPARE(matrix->GetElement(1,0), 0.);
  //transformsWidget->show();
  //qApp->exec();
}

// ----------------------------------------------------------------------------
void qSlicerTransformsModuleWidgetTester::testInvert()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLTransformNode> transformNode;
  scene->AddNode(transformNode.GetPointer());

  qSlicerTransformsModule transformsModule;
  transformsModule.setMRMLScene(scene.GetPointer());
  transformsModule.logic();
  qSlicerTransformsModuleWidget* transformsWidget =
    dynamic_cast<qSlicerTransformsModuleWidget*>(transformsModule.widgetRepresentation());

  vtkNew<vtkMatrix4x4> matrix;
  transformNode->GetMatrixTransformToParent(matrix.GetPointer());
  matrix->SetElement(0,0, 10.);
  matrix->SetElement(1,0, 2.);
  transformNode->SetMatrixTransformToParent(matrix.GetPointer());
  transformsWidget->invert();
  transformNode->GetMatrixTransformToParent(matrix.GetPointer());
  QCOMPARE(matrix->GetElement(0,0), 0.1);
  QCOMPARE(matrix->GetElement(1,0), -0.2);
  //transformsWidget->show();
  //qApp->exec();
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qSlicerTransformsModuleWidgetTest)
#include "moc_qSlicerTransformsModuleWidgetTest.cxx"
