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
#include <vtkMRMLColorLogic.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkStringArray.h>

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

  void testSetLabelVolumeWithNoLinkedControl();

  void testUpdateSliceOrientationSelector();
};

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::init()
{
  this->MRMLScene = vtkMRMLScene::New();

  // Add default color nodes
  vtkNew<vtkMRMLColorLogic> colorLogic;
  colorLogic->SetMRMLScene(this->MRMLScene);
  // need to set it back to nullptr, otherwise the logic removes the nodes that it added when it is destructed
  colorLogic->SetMRMLScene(nullptr);

  vtkNew<vtkMRMLSliceNode> sliceNode;
  sliceNode->SetLayoutName("Red");
  vtkNew<vtkMatrix3x3> axialSliceToRAS;
  vtkMRMLSliceNode::InitializeAxialMatrix(axialSliceToRAS.GetPointer());

  sliceNode->AddSliceOrientationPreset("Axial", axialSliceToRAS.GetPointer());
  sliceNode->SetOrientation("Axial");
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

  vtkNew<vtkMRMLLabelMapVolumeNode> labelmapNode1;
  labelmapNode1->SetName("Labelmap 1");
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
  void* nullPtr = nullptr;
  QCOMPARE(reinterpret_cast<void*>(sliceControllerWidget.mrmlScene()), nullPtr);
  QCOMPARE(reinterpret_cast<void*>(sliceControllerWidget.mrmlSliceNode()), nullPtr);

  // \todo sliceViewName is not consistent with label and color
  // Here is should compare to QString()
  QCOMPARE(sliceControllerWidget.sliceViewName(), QString("Red"));
  QCOMPARE(sliceControllerWidget.sliceViewLabel(), QString());
  QCOMPARE(sliceControllerWidget.sliceViewColor(), QColor());

  QVERIFY(sliceControllerWidget.sliceLogic() != nullptr);
  QCOMPARE(sliceControllerWidget.imageDataConnection(), nullPtr);
  QCOMPARE(sliceControllerWidget.mrmlSliceCompositeNode(), nullPtr);

  QCOMPARE(sliceControllerWidget.isLinked(), false);
  QCOMPARE(sliceControllerWidget.isCompareView(), false);
  QCOMPARE(sliceControllerWidget.sliceOrientation(), QString("Axial"));
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testSetMRMLSliceNode()
{
  qMRMLSliceControllerWidget sliceControllerWidget;
  void* nullPtr = nullptr;

  sliceControllerWidget.setSliceViewLabel("R");
  QCOMPARE(sliceControllerWidget.sliceViewLabel(), QString(""));

  sliceControllerWidget.setSliceViewColor(Qt::red);

  sliceControllerWidget.setMRMLScene(this->MRMLScene);
  sliceControllerWidget.setMRMLSliceNode(this->MRMLSliceNode);

  QCOMPARE(sliceControllerWidget.mrmlScene(), this->MRMLScene);
  QCOMPARE(sliceControllerWidget.mrmlSliceNode(), this->MRMLSliceNode);
  QVERIFY(sliceControllerWidget.sliceLogic() != nullptr);
  QCOMPARE(sliceControllerWidget.imageDataConnection(), nullPtr);

  QCOMPARE(sliceControllerWidget.mrmlSliceCompositeNode()->GetBackgroundVolumeID(), nullPtr);
  QCOMPARE(sliceControllerWidget.mrmlSliceCompositeNode()->GetForegroundVolumeID(), nullPtr);
  QCOMPARE(sliceControllerWidget.mrmlSliceCompositeNode()->GetLabelVolumeID(), nullPtr);

  QCOMPARE(sliceControllerWidget.sliceViewName(), QString("Red"));

  sliceControllerWidget.setSliceViewLabel("R");
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
  vtkMRMLNode* volumeNode = this->MRMLScene->GetNodeByID(volumeNodeID.toUtf8());
  sliceControllerWidget.mrmlSliceCompositeNode()->SetBackgroundVolumeID(volumeNode ? volumeNode->GetID() : nullptr);

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
  QTest::newRow("labelmap1") << "vtkMRMLLabelMapVolumeNode1" << "vtkMRMLLabelMapVolumeNode1";
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testSetForegroundVolume()
{
  qMRMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setMRMLScene(this->MRMLScene);

  QFETCH(QString, volumeNodeID);
  vtkMRMLNode* volumeNode = this->MRMLScene->GetNodeByID(volumeNodeID.toUtf8());
  sliceControllerWidget.mrmlSliceCompositeNode()->SetForegroundVolumeID(volumeNode ? volumeNode->GetID() : nullptr);

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
  QTest::newRow("labelmap1") << "vtkMRMLLabelMapVolumeNode1" << "vtkMRMLLabelMapVolumeNode1";
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testSetLabelVolume()
{
  qMRMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setMRMLScene(this->MRMLScene);

  QFETCH(QString, volumeNodeID);
  vtkMRMLNode* volumeNode = this->MRMLScene->GetNodeByID(volumeNodeID.toUtf8());
  sliceControllerWidget.mrmlSliceCompositeNode()->SetLabelVolumeID(volumeNode ? volumeNode->GetID() : nullptr);

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
  QTest::newRow("labelmap1") << "vtkMRMLLabelMapVolumeNode1" << "vtkMRMLLabelMapVolumeNode1";
}

// ----------------------------------------------------------------------------
void qMRMLSliceControllerWidgetTester::testSetLabelVolumeWithNoLinkedControl()
{
  qMRMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setMRMLScene(this->MRMLScene);

  vtkMRMLScalarVolumeNode* scalarVolumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast(this->MRMLScene->GetNodeByID("vtkMRMLLabelMapVolumeNode1"));
  if (scalarVolumeNode)
    {
    sliceControllerWidget.mrmlSliceCompositeNode()->SetLabelVolumeID(scalarVolumeNode->GetID());
    }

  vtkNew<vtkMRMLLabelMapVolumeNode> labelmapNode2;
  labelmapNode2->SetName("Labelmap 2");
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
void qMRMLSliceControllerWidgetTester::testUpdateSliceOrientationSelector()
{
  qMRMLSliceControllerWidget sliceControllerWidget;
  sliceControllerWidget.setSliceViewLabel("R");
  sliceControllerWidget.setSliceViewColor(Qt::red);
  sliceControllerWidget.setMRMLScene(this->MRMLScene);
  sliceControllerWidget.setMRMLSliceNode(this->MRMLSliceNode);
  QCOMPARE(sliceControllerWidget.sliceOrientation(), QString("Axial"));

  // Update the sliceToRAS matrix
  vtkMatrix4x4* sliceToRAS =
      sliceControllerWidget.mrmlSliceNode()->GetSliceToRAS();
  sliceToRAS->SetElement(0, 0, 1.2);
  sliceControllerWidget.mrmlSliceNode()->UpdateMatrices();

  // Make sure the presets have not been updated
  vtkNew<vtkStringArray> orientationNames;
  sliceControllerWidget.mrmlSliceNode()->GetSliceOrientationPresetNames(orientationNames.GetPointer());
  QCOMPARE(orientationNames->GetNumberOfValues(), static_cast<vtkIdType>(1));
  QCOMPARE(orientationNames->GetValue(0).c_str(), "Axial");

  // Check that current orientation is updated
  QCOMPARE(sliceControllerWidget.mrmlSliceNode()->GetOrientation(), std::string("Reformat"));
  QCOMPARE(sliceControllerWidget.sliceOrientation(), QString("Reformat"));

  // Check that "Reformat" is the last item in the selector
  QComboBox* orientationSelector =
      sliceControllerWidget.findChild<QComboBox*>("SliceOrientationSelector");
  QVERIFY(orientationSelector != nullptr);
  QStringList items;
  for(int idx = 0; idx < orientationSelector->count(); ++idx)
    {
    items << orientationSelector->itemText(idx);
    }
  QCOMPARE(items, QStringList() << "Axial" << "Reformat");

  // Set orientation back to "Axial"
  sliceControllerWidget.mrmlSliceNode()->SetOrientation("Axial");
  QCOMPARE(sliceControllerWidget.sliceOrientation(), QString("Axial"));

}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLSliceControllerWidgetTest)
#include "moc_qMRMLSliceControllerWidgetTest.cxx"
