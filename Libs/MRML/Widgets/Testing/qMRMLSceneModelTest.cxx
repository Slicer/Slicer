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

// CTK includes
#include "ctkTest.h"

// MRML includes
#include "qMRMLSceneFactoryWidget.h"
#include "qMRMLSceneModel.h"
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkNew.h>

// ----------------------------------------------------------------------------
class qMRMLSceneModelTester: public QObject
{
  Q_OBJECT
  void setColumns(qMRMLSceneModel& model);
private slots:
  void testDefaults();
  void testSetsAndGets();
  void testSetScene();
  void testSetColumns();
  void testSetColumns_data();
  void testSetColumnsWithScene();
  void testSetColumnsWithScene_data();
};

// ----------------------------------------------------------------------------
void qMRMLSceneModelTester::testDefaults()
{
  qMRMLSceneModel sceneModel;
  QCOMPARE(sceneModel.listenNodeModifiedEvent(), qMRMLSceneModel::OnlyVisibleNodes);
  QCOMPARE(sceneModel.lazyUpdate(), false);
  QCOMPARE(sceneModel.nameColumn(), 0);
  QCOMPARE(sceneModel.idColumn(), -1);
  QCOMPARE(sceneModel.checkableColumn(), -1);
  QCOMPARE(sceneModel.visibilityColumn(), -1);
  QCOMPARE(sceneModel.toolTipNameColumn(), -1);

  QVERIFY(sceneModel.mrmlScene() == nullptr);
  QVERIFY(sceneModel.mrmlSceneItem() == nullptr);
  QVERIFY(!sceneModel.mrmlSceneIndex().isValid());
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelTester::testSetsAndGets()
{
  qMRMLSceneModel sceneModel;

  sceneModel.setListenNodeModifiedEvent(qMRMLSceneModel::NoNodes);
  QCOMPARE(sceneModel.listenNodeModifiedEvent(), qMRMLSceneModel::NoNodes);

  sceneModel.setListenNodeModifiedEvent(qMRMLSceneModel::AllNodes);
  QCOMPARE(sceneModel.listenNodeModifiedEvent(), qMRMLSceneModel::AllNodes);

  sceneModel.setLazyUpdate(true);
  QCOMPARE(sceneModel.lazyUpdate(), true);

  sceneModel.setLazyUpdate(false);
  QCOMPARE(sceneModel.lazyUpdate(), false);

  vtkNew<vtkMRMLScene> scene;
  sceneModel.setMRMLScene(scene.GetPointer());
  QCOMPARE(sceneModel.mrmlScene(), scene.GetPointer());
}


// ----------------------------------------------------------------------------
void qMRMLSceneModelTester::testSetScene()
{
  qMRMLSceneModel sceneModel;
  vtkNew<vtkMRMLScene> scene;
  sceneModel.setMRMLScene(scene.GetPointer());
  QVERIFY(sceneModel.mrmlSceneItem() != nullptr);
  QVERIFY(sceneModel.mrmlSceneIndex().isValid());
  QCOMPARE(sceneModel.mrmlSceneIndex().row(), 0);
  QCOMPARE(sceneModel.mrmlSceneIndex().column(), 0);
  QVERIFY(!sceneModel.mrmlSceneIndex().parent().isValid());
  QCOMPARE(sceneModel.columnCount(), 1);
  QCOMPARE(sceneModel.columnCount(sceneModel.mrmlSceneIndex()), 1);
  vtkNew<vtkMRMLViewNode> node;
  scene->AddNode(node.GetPointer());
  QCOMPARE(sceneModel.columnCount(), 1);
  QCOMPARE(sceneModel.columnCount(sceneModel.mrmlSceneIndex()), 1);
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelTester::testSetColumns()
{
  qMRMLSceneModel sceneModel;
  this->setColumns(sceneModel);
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelTester::setColumns(qMRMLSceneModel& sceneModel)
{
  QFETCH(int, nameColumn);
  QFETCH(int, idColumn);
  QFETCH(int, checkableColumn);
  QFETCH(int, visibilityColumn);
  QFETCH(int, toolTipNameColumn);

  sceneModel.setNameColumn(nameColumn);
  QCOMPARE(sceneModel.nameColumn(), nameColumn);

  sceneModel.setIDColumn(idColumn);
  QCOMPARE(sceneModel.idColumn(), idColumn);

  sceneModel.setCheckableColumn(checkableColumn);
  QCOMPARE(sceneModel.checkableColumn(), checkableColumn);

  sceneModel.setVisibilityColumn(visibilityColumn);
  QCOMPARE(sceneModel.visibilityColumn(), visibilityColumn);

  sceneModel.setToolTipNameColumn(toolTipNameColumn);
  QCOMPARE(sceneModel.toolTipNameColumn(), toolTipNameColumn);
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelTester::testSetColumns_data()
{
  QTest::addColumn<int>("nameColumn");
  QTest::addColumn<int>("idColumn");
  QTest::addColumn<int>("checkableColumn");
  QTest::addColumn<int>("visibilityColumn");
  QTest::addColumn<int>("toolTipNameColumn");
  QTest::addColumn<int>("extraItemColumn");
  QTest::addColumn<int>("columnCount");


  QTest::newRow("name") << 0 << -1 << -1 << -1 << -1 << -1 << 1;
  QTest::newRow("checkable name") << 0 << -1 << 0 << -1 << -1 << -1 << 1;
  QTest::newRow("visible name") << 0 << -1 << -1 << 0 << -1 << -1 << 1;
  QTest::newRow("none") << -1 << -1 << -1 << -1 << -1 << -1 << 1;
  QTest::newRow("all 0") << 0 << 0 << 0 << 0 << 0 << 0 << 1;
  QTest::newRow("id") << -1 << 0 << -1 << -1 << -1 << -1 << 1;
  QTest::newRow("id + tooltip") << -1 << 0 << -1 << -1 << 0 << -1 << 1;
  QTest::newRow("checkable id") << -1 << 0 << 0 << -1 << -1 << -1 << 1;
  QTest::newRow("visible id") << -1 << 0 << -1 << 0 << -1 << -1 << 1;
  QTest::newRow("name + id") << 0 << 1 << -1 << -1 << -1 << -1 << 2;
  QTest::newRow("checkable name + id") << 0 << 1 << 0 << -1 << -1 << -1 << 2;
  QTest::newRow("checkable + name + id") << 1 << 2 << 0 << -1 << -1 << -1 << 3;
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelTester::testSetColumnsWithScene()
{
  qMRMLSceneModel sceneModel;

  qMRMLSceneFactoryWidget sceneFactory;
  sceneFactory.generateScene();
  sceneModel.setMRMLScene(sceneFactory.mrmlScene());

  for (int i=0; i < 100; ++i)
    {
    sceneFactory.generateNode();
    }

  this->setColumns(sceneModel);
  QFETCH(int, columnCount);
  QCOMPARE(sceneModel.columnCount(), columnCount);
  QCOMPARE(sceneModel.columnCount(sceneModel.mrmlSceneIndex()), columnCount);
}

// ----------------------------------------------------------------------------
void qMRMLSceneModelTester::testSetColumnsWithScene_data()
{
  this->testSetColumns_data();
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLSceneModelTest)
#include "moc_qMRMLSceneModelTest.cxx"
