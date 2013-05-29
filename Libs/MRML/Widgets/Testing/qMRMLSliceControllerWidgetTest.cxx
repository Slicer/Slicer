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
#include "qMRMLNodeComboBox.h"
#include "qMRMLSliceControllerWidget.h"
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkNew.h>

// ----------------------------------------------------------------------------
class qMRMLSliceControllerWidgetTester: public QObject
{
  Q_OBJECT
  vtkMRMLScene* MRMLScene;
  vtkMRMLSliceNode* MRMLSliceNode;

private slots:
  /// Run before each test
  void init();

  /// Run after each test
  void cleanup();

  void testDefaults();
  void testSetMRMLSliceNode();

  void testSetBackgroundVolume();
  void testSetBackgroundVolume_data();

  void testSetForegroundVolume();
  void testSetForegroundVolume_data();

  void testSetLabelVolume();
  void testSetLabelVolume_data();

  void testChangeLabelMapToScalarVolume();
  void testSetLabelVolumeWithNoLinkedControl();
};

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::init()
{
  this->MRMLScene = vtkMRMLScene::New();

  vtkNew<vtkMRMLSliceNode> sliceNode;
  sliceNode->SetLayoutName("Red");
  this->MRMLScene->AddNode(sliceNode.GetPointer());

  vtkNew<vtkMRMLScalarVolumeNode> volumeNode1;
  volumeNode1->SetName("Volume 1");
  this->MRMLScene->AddNode(volumeNode1.GetPointer());

  vtkNew<vtkMRMLScalarVolumeNode> volumeNode2;
  volumeNode2->SetName("Volume 2");
  this->MRMLScene->AddNode(volumeNode2.GetPointer());

  vtkNew<vtkMRMLScalarVolumeNode> volumeNode3;
  volumeNode3->SetName("Volume 3");
  this->MRMLScene->AddNode(volumeNode3.GetPointer());

  vtkNew<vtkMRMLScalarVolumeNode> labelmapNode1;
  labelmapNode1->SetName("Labelmap 1");
  labelmapNode1->SetLabelMap(1);
  this->MRMLScene->AddNode(labelmapNode1.GetPointer());

  this->MRMLSliceNode = sliceNode.GetPointer();
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::cleanup()
{
  this->MRMLScene->Delete();
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testDefaults()
{
  qMRMLSliceControllerWidget sliceControllerWidget;
  void* nullPtr = 0;
  QCOMPARE(reinterpret_cast<void*>(sliceControllerWidget.mrmlScene()), nullPtr);
  QCOMPARE(reinterpret_cast<void*>(sliceControllerWidget.mrmlSliceNode()), nullPtr);

  // \todo sliceViewName is not consistent with label and color
  // Here is should compare to QString()
  QCOMPARE(sliceControllerWidget.sliceViewName(), QString("Red"));
  QCOMPARE(sliceControllerWidget.sliceViewLabel(), QString());
  QCOMPARE(sliceControllerWidget.sliceViewColor(), QColor());

  QVERIFY(sliceControllerWidget.sliceLogic() != 0);
  QCOMPARE(sliceControllerWidget.imageData(), nullPtr);
  QCOMPARE(sliceControllerWidget.mrmlSliceCompositeNode(), nullPtr);

  QCOMPARE(sliceControllerWidget.isLinked(), false);
  QCOMPARE(sliceControllerWidget.isCompareView(), false);
  QCOMPARE(sliceControllerWidget.sliceOrientation(), QString("Axial"));
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testSetMRMLSliceNode()
{
  qMRMLSliceControllerWidget sliceControllerWidget;
  void* nullPtr = 0;

  // \todo The following should be set automatically from setMRMLSliceNode
  sliceControllerWidget.setSliceViewLabel("R");
  sliceControllerWidget.setSliceViewColor(Qt::red);

  sliceControllerWidget.setMRMLScene(this->MRMLScene);
  sliceControllerWidget.setMRMLSliceNode(this->MRMLSliceNode);

  QCOMPARE(sliceControllerWidget.mrmlScene(), this->MRMLScene);
  QCOMPARE(sliceControllerWidget.mrmlSliceNode(), this->MRMLSliceNode);
  QVERIFY(sliceControllerWidget.sliceLogic() != 0);
  QCOMPARE(sliceControllerWidget.imageData(), nullPtr);

  QCOMPARE(sliceControllerWidget.mrmlSliceCompositeNode()->GetBackgroundVolumeID(), nullPtr);
  QCOMPARE(sliceControllerWidget.mrmlSliceCompositeNode()->GetForegroundVolumeID(), nullPtr);
  QCOMPARE(sliceControllerWidget.mrmlSliceCompositeNode()->GetLabelVolumeID(), nullPtr);

  QCOMPARE(sliceControllerWidget.sliceViewName(), QString("Red"));
  QCOMPARE(sliceControllerWidget.sliceViewLabel(), QString("R"));
  QCOMPARE(sliceControllerWidget.sliceViewColor(), QColor(Qt::red));

  QCOMPARE(sliceControllerWidget.isLinked(), false);
  QCOMPARE(sliceControllerWidget.isCompareView(), false);
  QCOMPARE(sliceControllerWidget.sliceOrientation(), QString("Axial"));
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testSetBackgroundVolume()
{
  qMRMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setMRMLScene(this->MRMLScene);

  QFETCH(QString, volumeNodeID);
  vtkMRMLNode* volumeNode = this->MRMLScene->GetNodeByID(volumeNodeID.toLatin1());
  sliceControllerWidget.mrmlSliceCompositeNode()->SetBackgroundVolumeID(volumeNode ? volumeNode->GetID() : 0);

  QFETCH(QString, expectedVolumeNodeID);
  qMRMLNodeComboBox* comboBox =
    qobject_cast<qMRMLNodeComboBox*>(sliceControllerWidget.findChild<qMRMLNodeComboBox*>("BackgroundComboBox"));
  QCOMPARE(comboBox->currentNodeID(), expectedVolumeNodeID);

  //sliceControllerWidget.show();
  //qApp->exec();
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testSetBackgroundVolume_data()
{
  QTest::addColumn<QString>("volumeNodeID");
  QTest::addColumn<QString>("expectedVolumeNodeID");

  QTest::newRow("no node") << QString() << QString();
  QTest::newRow("volume1") << "vtkMRMLScalarVolumeNode1" << "vtkMRMLScalarVolumeNode1";
  QTest::newRow("volume2") << "vtkMRMLScalarVolumeNode2" << "vtkMRMLScalarVolumeNode2";
  QTest::newRow("volume3") << "vtkMRMLScalarVolumeNode3" << "vtkMRMLScalarVolumeNode3";
  QTest::newRow("labelmap1") << "vtkMRMLScalarVolumeNode4" << "vtkMRMLScalarVolumeNode4";
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testSetForegroundVolume()
{
  qMRMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setMRMLScene(this->MRMLScene);

  QFETCH(QString, volumeNodeID);
  vtkMRMLNode* volumeNode = this->MRMLScene->GetNodeByID(volumeNodeID.toLatin1());
  sliceControllerWidget.mrmlSliceCompositeNode()->SetForegroundVolumeID(volumeNode ? volumeNode->GetID() : 0);

  QFETCH(QString, expectedVolumeNodeID);
  qMRMLNodeComboBox* comboBox =
    qobject_cast<qMRMLNodeComboBox*>(sliceControllerWidget.findChild<qMRMLNodeComboBox*>("ForegroundComboBox"));
  QCOMPARE(comboBox->currentNodeID(), expectedVolumeNodeID);

  //sliceControllerWidget.show();
  //qApp->exec();
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testSetForegroundVolume_data()
{
  QTest::addColumn<QString>("volumeNodeID");
  QTest::addColumn<QString>("expectedVolumeNodeID");

  QTest::newRow("no node") << QString() << QString();
  QTest::newRow("volume1") << "vtkMRMLScalarVolumeNode1" << "vtkMRMLScalarVolumeNode1";
  QTest::newRow("volume2") << "vtkMRMLScalarVolumeNode2" << "vtkMRMLScalarVolumeNode2";
  QTest::newRow("volume3") << "vtkMRMLScalarVolumeNode3" << "vtkMRMLScalarVolumeNode3";
  QTest::newRow("labelmap1") << "vtkMRMLScalarVolumeNode4" << "vtkMRMLScalarVolumeNode4";
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testSetLabelVolume()
{
  qMRMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setMRMLScene(this->MRMLScene);

  QFETCH(QString, volumeNodeID);
  vtkMRMLNode* volumeNode = this->MRMLScene->GetNodeByID(volumeNodeID.toLatin1());
  sliceControllerWidget.mrmlSliceCompositeNode()->SetLabelVolumeID(volumeNode ? volumeNode->GetID() : 0);

  QFETCH(QString, expectedVolumeNodeID);
  qMRMLNodeComboBox* comboBox =
    qobject_cast<qMRMLNodeComboBox*>(sliceControllerWidget.findChild<qMRMLNodeComboBox*>("LabelMapComboBox"));
  QCOMPARE(comboBox->currentNodeID(), expectedVolumeNodeID);

  //sliceControllerWidget.show();
  //qApp->exec();
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testSetLabelVolume_data()
{
  QTest::addColumn<QString>("volumeNodeID");
  QTest::addColumn<QString>("expectedVolumeNodeID");

  QTest::newRow("no node") << QString() << QString();
  QTest::newRow("volume1") << "vtkMRMLScalarVolumeNode1" << QString();
  QTest::newRow("volume2") << "vtkMRMLScalarVolumeNode2" << QString();
  QTest::newRow("volume3") << "vtkMRMLScalarVolumeNode3" << QString();
  QTest::newRow("labelmap1") << "vtkMRMLScalarVolumeNode4" << "vtkMRMLScalarVolumeNode4";
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testChangeLabelMapToScalarVolume()
{
  qMRMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setMRMLScene(this->MRMLScene);

  vtkMRMLScalarVolumeNode* scalarVolumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast(this->MRMLScene->GetNodeByID("vtkMRMLScalarVolumeNode4"));
  sliceControllerWidget.mrmlSliceCompositeNode()->SetLabelVolumeID("vtkMRMLScalarVolumeNode4");

  // Remove the label map property
  scalarVolumeNode->SetLabelMap(0);

  qMRMLNodeComboBox* comboBox =
    qobject_cast<qMRMLNodeComboBox*>(sliceControllerWidget.findChild<qMRMLNodeComboBox*>("LabelMapComboBox"));
  QCOMPARE(comboBox->currentNodeID(), QString());
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testSetLabelVolumeWithNoLinkedControl()
{
  qMRMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setMRMLScene(this->MRMLScene);

  vtkMRMLScalarVolumeNode* scalarVolumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast(this->MRMLScene->GetNodeByID("vtkMRMLScalarVolumeNode4"));
  if (scalarVolumeNode)
    {
    sliceControllerWidget.mrmlSliceCompositeNode()->SetLabelVolumeID(scalarVolumeNode->GetID());
    }

  vtkNew<vtkMRMLScalarVolumeNode> labelmapNode2;
  labelmapNode2->SetName("Labelmap 2");
  labelmapNode2->SetLabelMap(1);
  this->MRMLScene->AddNode(labelmapNode2.GetPointer());

  vtkMRMLSliceCompositeNode* sliceCompositeNode =
    sliceControllerWidget.mrmlSliceCompositeNode();

  sliceCompositeNode->SetLinkedControl(0);
  sliceControllerWidget.sliceLogic()->StartSliceCompositeNodeInteraction(
    vtkMRMLSliceCompositeNode::LabelVolumeFlag);
  sliceCompositeNode->SetLabelVolumeID(labelmapNode2->GetID());
  sliceControllerWidget.sliceLogic()->EndSliceCompositeNodeInteraction();
  sliceCompositeNode->SetLinkedControl(1);
  sliceCompositeNode->Modified();

  qMRMLNodeComboBox* comboBox =
    qobject_cast<qMRMLNodeComboBox*>(sliceControllerWidget.findChild<qMRMLNodeComboBox*>("LabelMapComboBox"));
  QCOMPARE(comboBox->currentNodeID(), QString(labelmapNode2->GetID()));

  //sliceControllerWidget.show();
  //qApp->exec();
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLSliceControllerWidgetTest)
#include "moc_qMRMLSliceControllerWidgetTest.cxx"
