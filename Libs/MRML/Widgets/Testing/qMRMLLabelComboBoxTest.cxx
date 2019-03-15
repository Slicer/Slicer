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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// CTK includes
#include <ctkTest.h>

// qMRML includes
#include "qMRMLLabelComboBox.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// --------------------------------------------------------------------------
class qMRMLLabelComboBoxTester: public QObject
{
  Q_OBJECT
private:

  vtkSmartPointer<vtkMRMLColorTableNode> ColorTableNode;

  qMRMLLabelComboBox * LabelComboBox;

private slots:
  void init();
  void cleanup();

  void testSetMRMLColorNode();

  void testSetNoneEnabled();

  void testSetCurrentColor();
  void testSetCurrentColor_data();

  void testSetColorNameVisible();
};

// ----------------------------------------------------------------------------
void qMRMLLabelComboBoxTester::init()
{
  this->ColorTableNode = vtkSmartPointer<vtkMRMLColorTableNode>::New();
  this->ColorTableNode->SetType(vtkMRMLColorTableNode::Labels);

  this->LabelComboBox = new qMRMLLabelComboBox();
}

// ----------------------------------------------------------------------------
void qMRMLLabelComboBoxTester::cleanup()
{
  if (this->LabelComboBox == nullptr)
    {
    return;
    }
  delete this->LabelComboBox;
}

// ----------------------------------------------------------------------------
void qMRMLLabelComboBoxTester::testSetMRMLColorNode()
{
  QVERIFY(this->LabelComboBox->mrmlColorNode() == nullptr);

  this->LabelComboBox->setMRMLColorNode(this->ColorTableNode);
  QCOMPARE(this->LabelComboBox->mrmlColorNode(), this->ColorTableNode.GetPointer());
}

// ----------------------------------------------------------------------------
void qMRMLLabelComboBoxTester::testSetNoneEnabled()
{
  QVERIFY(!this->LabelComboBox->noneEnabled());

  this->LabelComboBox->setNoneEnabled(true);
  QVERIFY(this->LabelComboBox->noneEnabled());

  this->LabelComboBox->setNoneEnabled(false);
  QVERIFY(!this->LabelComboBox->noneEnabled());
}

// ----------------------------------------------------------------------------
void qMRMLLabelComboBoxTester::testSetCurrentColor()
{
  QFETCH(bool, shouldSetColorNode);
  QFETCH(bool, noneEnabledValue);
  QFETCH(int, currentColorToSet);
  QFETCH(int, expectedCurrentColor);
  QFETCH(int, currentColorToSet2);
  QFETCH(int, expectedCurrentColor2);

  vtkMRMLColorNode* colorNodeToSet = nullptr;
  if (shouldSetColorNode)
    {
    colorNodeToSet = this->ColorTableNode;
    }

  QVERIFY(this->LabelComboBox->currentColor() == -1);

  this->LabelComboBox->setNoneEnabled(noneEnabledValue);
  this->LabelComboBox->setMRMLColorNode(colorNodeToSet);

  QVERIFY(this->LabelComboBox->currentColor() == -1);

  this->LabelComboBox->setCurrentColor(currentColorToSet);
  QCOMPARE(this->LabelComboBox->currentColor(), expectedCurrentColor);

  this->LabelComboBox->setCurrentColor(currentColorToSet2);
  QCOMPARE(this->LabelComboBox->currentColor(), expectedCurrentColor2);
}

// ----------------------------------------------------------------------------
void qMRMLLabelComboBoxTester::testSetCurrentColor_data()
{
  QTest::addColumn<bool>("shouldSetColorNode");
  QTest::addColumn<bool>("noneEnabledValue");
  QTest::addColumn<int>("currentColorToSet");
  QTest::addColumn<int>("expectedCurrentColor");
  QTest::addColumn<int>("currentColorToSet2");
  QTest::addColumn<int>("expectedCurrentColor2");

  // Check if currentColor()/setCurrentColor() work properly when
  // no ColorNode has been assigned  and NoneEnabled is false
  QTest::newRow("0") << false << false << 10 << -1 << 260 << -1;

  // Check if currentColor()/setCurrentColor() work properly when
  // a valid ColorNode has been assigned and NoneEnabled is false
  QTest::newRow("1") << true << false << 5 << 5 << -6 << 5;
  QTest::newRow("2") << true << false << 4 << 4 << -1 << 4;
  QTest::newRow("3") << true << false << 256 << 256 << 257 << 256;

  QTest::newRow("4") << true << true << 5 << 5 << -6 << 5;
  QTest::newRow("5") << true << true << 4 << 4 << -1 << -1;
  QTest::newRow("6") << true << true << 256 << 256 << 257 << 256;
}

// ----------------------------------------------------------------------------
void qMRMLLabelComboBoxTester::testSetColorNameVisible()
{
  this->LabelComboBox->setMRMLColorNode(this->ColorTableNode);

  QVERIFY(this->LabelComboBox->colorNameVisible());

  this->LabelComboBox->setColorNameVisible(false);
  QVERIFY(!this->LabelComboBox->colorNameVisible());

  this->LabelComboBox->setColorNameVisible(true);
  QVERIFY(this->LabelComboBox->colorNameVisible());
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLLabelComboBoxTest)
#include "moc_qMRMLLabelComboBoxTest.cxx"
