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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QItemSelectionModel>

// CTK includes
#include "ctkTest.h"

// MRML includes
#include "qMRMLNodeAttributeTableView.h"
#include <vtkMRMLModelNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// ----------------------------------------------------------------------------
class qMRMLNodeAttributeTableViewTester: public QObject
{
  Q_OBJECT
  qMRMLNodeAttributeTableView* NodeAttributeTableView;

private slots:
  void init();
  void cleanup();

  void testDefaults();
  void testPopulate();
  void testSelect();
  void testSetAttribute();
  void testSetAttribute_data();
  void testAdd();
  void testRemove();
};

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::init()
{
  this->NodeAttributeTableView = new qMRMLNodeAttributeTableView;
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::cleanup()
{
  QVERIFY(this->NodeAttributeTableView != NULL);
  this->NodeAttributeTableView->setInspectedNode(NULL);
  delete this->NodeAttributeTableView;
  this->NodeAttributeTableView = NULL;
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testDefaults()
{
  QCOMPARE((this->NodeAttributeTableView->inspectedNode() == NULL), true);
  QCOMPARE(this->NodeAttributeTableView->attributeCount(), 0);
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testPopulate()
{
  this->NodeAttributeTableView->setInspectedNode(NULL);
  QCOMPARE(this->NodeAttributeTableView->attributeCount(), 0);

  vtkNew<vtkMRMLModelNode> node;
  node->SetAttribute("Attribute1", "Value1");
  node->SetAttribute("Attribute2", "Value2");
  this->NodeAttributeTableView->setInspectedNode(node.GetPointer());
  QCOMPARE(this->NodeAttributeTableView->attributeCount(), 2);

  node->SetAttribute("Attribute3", "Value3");
  QCOMPARE(this->NodeAttributeTableView->attributeCount(), 3);

  this->NodeAttributeTableView->setInspectedNode(NULL);
  QCOMPARE(this->NodeAttributeTableView->attributeCount(), 0);
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testSetAttribute()
{
  vtkNew<vtkMRMLModelNode> node;
  node->SetAttribute("Attribute1", "Value1");
  this->NodeAttributeTableView->setInspectedNode(node.GetPointer());

  QFETCH(bool, setOnNode);
  QFETCH(QString, attribute);
  QFETCH(QString, value);
  if (setOnNode)
    {
    node->SetAttribute(attribute.isNull() ? QByteArray(0) : attribute.toLatin1(),
                       value.isNull() ? QByteArray(0) : value.toLatin1());
    }
  else
    {
    this->NodeAttributeTableView->setAttribute(attribute, value);
    }

  QFETCH(int, expectedAttributeCount);
  QFETCH(QString, expectedAttribute1Value);
  if (setOnNode)
    {
    QCOMPARE(this->NodeAttributeTableView->attributeCount(),
             expectedAttributeCount);
    QCOMPARE(this->NodeAttributeTableView->attributeValue("Attribute1"),
             expectedAttribute1Value);
    }
  else
    {
    QCOMPARE(node->GetAttributeNames().size(), static_cast<size_t>(expectedAttributeCount));
    QCOMPARE(QString(node->GetAttribute("Attribute1")), expectedAttribute1Value);
    }
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testSetAttribute_data()
{
  QTest::addColumn<bool>("setOnNode");
  QTest::addColumn<QString>("attribute");
  QTest::addColumn<QString>("value");
  QTest::addColumn<int>("expectedAttributeCount");
  QTest::addColumn<QString>("expectedAttribute1Value");

  for (int setOnNode = 0; setOnNode < 2; ++setOnNode)
    {
    // null vs empty:
    //   QString makes a difference between a QString() and QString("")
    //   vtkMRMLNode::SetAttribute() makes a difference between 0 and ""
    QTest::newRow("null null") << (setOnNode != 0) << QString() << QString() << 1 << "Value1";
    QTest::newRow("null empty") << (setOnNode != 0) << QString() << QString("") << 1 << "Value1";
    QTest::newRow("null valid") << (setOnNode != 0) << QString() << QString("Value2") << 1 << "Value1";
    QTest::newRow("empty null") << (setOnNode != 0) << QString("") << QString() << 1 << "Value1";
    QTest::newRow("empty empty") << (setOnNode != 0) << QString("") << QString("") << 1 << "Value1";
    QTest::newRow("empty valid") << (setOnNode != 0) << QString("") << QString("Value2") << 1 << "Value1";
    QTest::newRow("valid null") << (setOnNode != 0) << QString("Attribute2") << QString() << 1 << "Value1";
    QTest::newRow("valid empty") << (setOnNode != 0) << QString("Attribute2") << QString("") << 2 << "Value1";
    QTest::newRow("valid valid") << (setOnNode != 0) << QString("Attribute2") << QString("Value2") << 2 << "Value1";

    QTest::newRow("replace null") << (setOnNode != 0) << QString("Attribute1") << QString() << 0 << QString();
    QTest::newRow("replace empty") << (setOnNode != 0) << QString("Attribute1") << QString("") << 1 << QString("");
    QTest::newRow("replace valid") << (setOnNode != 0) << QString("Attribute1") << QString("Value2") << 1 << QString("Value2");
  }
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testSelect()
{
  vtkNew<vtkMRMLModelNode> node;
  node->SetAttribute("Attribute1", "Value1");
  node->SetAttribute("Attribute2", "Value2");
  this->NodeAttributeTableView->setInspectedNode(node.GetPointer());
  this->NodeAttributeTableView->selectItemRange(1,0,1,0);
  QCOMPARE(this->NodeAttributeTableView->selectionModel()->selectedIndexes().count(), 1);
  QCOMPARE(this->NodeAttributeTableView->selectionModel()->selection().at(0).top(), 1);
  QCOMPARE(this->NodeAttributeTableView->selectionModel()->selection().at(0).left(), 0);
  QCOMPARE(this->NodeAttributeTableView->selectionModel()->selection().at(0).bottom(), 1);
  QCOMPARE(this->NodeAttributeTableView->selectionModel()->selection().at(0).right(), 0);

  this->NodeAttributeTableView->setInspectedNode(NULL);
  QCOMPARE(this->NodeAttributeTableView->selectionModel()->selectedIndexes().count(), 0);
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testAdd()
{
  vtkNew<vtkMRMLModelNode> node;
  node->SetAttribute("Attribute1", "Value1");
  this->NodeAttributeTableView->setInspectedNode(node.GetPointer());
  this->NodeAttributeTableView->addAttribute();
  QCOMPARE(this->NodeAttributeTableView->attributeCount(), 2);

  this->NodeAttributeTableView->renameAttribute(QString(""), QString("Attribute2"));
  QCOMPARE((int)this->NodeAttributeTableView->inspectedNode()->GetAttributeNames().size(), 2);
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testRemove()
{
  vtkNew<vtkMRMLModelNode> node;
  node->SetAttribute("Attribute1", "Value1");
  node->SetAttribute("Attribute2", "Value2");
  node->SetAttribute("Attribute3", "Value3");
  this->NodeAttributeTableView->setInspectedNode(node.GetPointer());
  this->NodeAttributeTableView->selectItemRange(1,0,1,1);
  this->NodeAttributeTableView->removeSelectedAttributes();
  QCOMPARE(this->NodeAttributeTableView->attributeCount(), 2);
  QCOMPARE(this->NodeAttributeTableView->attributeValue(QString("Attribute1")).isEmpty(), false);
  QCOMPARE(this->NodeAttributeTableView->attributeValue(QString("Attribute2")).isEmpty(), true);
  QCOMPARE(this->NodeAttributeTableView->attributeValue(QString("Attribute3")).isEmpty(), false);
  QCOMPARE((int)this->NodeAttributeTableView->inspectedNode()->GetAttribute("Attribute2"), NULL);

}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLNodeAttributeTableViewTest)
#include "moc_qMRMLNodeAttributeTableViewTest.cxx"
