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
