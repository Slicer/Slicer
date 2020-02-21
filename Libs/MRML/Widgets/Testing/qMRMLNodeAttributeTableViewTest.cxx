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

// Common test driver includes
#include "qMRMLWidgetCxxTests.h"

// ----------------------------------------------------------------------------
class qMRMLNodeAttributeTableViewTester: public QObject
{
  Q_OBJECT
  qMRMLNodeAttributeTableView* NodeAttributeTableView;

public:
  typedef QPair<QString, QString> AttributeType;
  typedef QPair<QString, bool> AttributeEmptyType;

private slots:
  void init();
  void cleanup();

  void testDefaults();

  void testPopulate();
  void testPopulate_data();

  void testSelect();
  void testSelect_data();

  void testSetAttribute();
  void testSetAttribute_data();

  void testAdd();
  void testAdd_data();

  void testRename();
  void testRename_data();

  void testRemove();
  void testRemove_data();

  void testSelectAndAdd();
  void testSelectAndAdd_data();
};

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(qMRMLNodeAttributeTableViewTester::AttributeType)
Q_DECLARE_METATYPE(QList<qMRMLNodeAttributeTableViewTester::AttributeType>)
Q_DECLARE_METATYPE(qMRMLNodeAttributeTableViewTester::AttributeEmptyType)
Q_DECLARE_METATYPE(QList<qMRMLNodeAttributeTableViewTester::AttributeEmptyType>)

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::init()
{
  this->NodeAttributeTableView = new qMRMLNodeAttributeTableView;
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::cleanup()
{
  QVERIFY(this->NodeAttributeTableView != nullptr);
  this->NodeAttributeTableView->setInspectedNode(nullptr);
  delete this->NodeAttributeTableView;
  this->NodeAttributeTableView = nullptr;
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testDefaults()
{
  QCOMPARE((this->NodeAttributeTableView->inspectedNode() == nullptr), true);
  QCOMPARE(this->NodeAttributeTableView->attributeCount(), 0);
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testPopulate()
{
  // We may get empty name, in that case error is logged
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN();

  vtkNew<vtkMRMLModelNode> node;

  QFETCH(QList<AttributeType>, attributes);
  foreach(const AttributeType& attribute, attributes)
    {
    node->SetAttribute(attribute.first.toUtf8(), attribute.second.toUtf8());
    }

  this->NodeAttributeTableView->setInspectedNode(node.GetPointer());

  QFETCH(QList<AttributeType>, expectedAttributes);
  QCOMPARE(static_cast<int>(node->GetAttributeNames().size()), expectedAttributes.count());
  QCOMPARE(this->NodeAttributeTableView->attributeCount(), expectedAttributes.count());

  QStringList resultAttributes = this->NodeAttributeTableView->attributes();
  int i = 0;
  foreach (QString attribute, resultAttributes)
    {
    QCOMPARE(attribute, expectedAttributes[i].first);
    QCOMPARE(this->NodeAttributeTableView->attributeValue(attribute),
             expectedAttributes[i].second);
    ++i;
    }

  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testPopulate_data()
{
  QTest::addColumn<QList<AttributeType> >("attributes");
  QTest::addColumn<QList<AttributeType> >("expectedAttributes");

  QTest::newRow("empty")
    << QList<AttributeType>() << QList<AttributeType>();
  QTest::newRow("1 attribute")
    << ( QList<AttributeType>() << AttributeType("Attribute1", "Value1"))
    << ( QList<AttributeType>() << AttributeType("Attribute1", "Value1"));
  QTest::newRow("2 attributes")
    << ( QList<AttributeType>()
         << AttributeType("Attribute1", "Value1")
         << AttributeType("Attribute2", "Value2") )
    << ( QList<AttributeType>()
         << AttributeType("Attribute1", "Value1")
         << AttributeType("Attribute2", "Value2") );
  QTest::newRow("2 same values")
    << ( QList<AttributeType>()
         << AttributeType("Attribute1", "Value1")
         << AttributeType("Attribute1", "Value1") )
    << ( QList<AttributeType>() << AttributeType("Attribute1", "Value1") );
  QTest::newRow("2 same attributes")
    << ( QList<AttributeType>()
         << AttributeType("Attribute1", "Value1")
         << AttributeType("Attribute1", "Value2") )
    << ( QList<AttributeType>() << AttributeType("Attribute1", "Value2") );
  QTest::newRow("2 empty values")
     << ( QList<AttributeType>()
          << AttributeType("Attribute1", "")
          << AttributeType("Attribute2", "") )
     << ( QList<AttributeType>()
          << AttributeType("Attribute1", "")
          << AttributeType("Attribute2", "") );
  QTest::newRow("2 empty attributes/values")
    << ( QList<AttributeType>()
         << AttributeType("", "")
         << AttributeType("", "") )
    << ( QList<AttributeType>() );
  QTest::newRow("2 empty attributes")
    << ( QList<AttributeType>()
         << AttributeType("", "Value1")
         << AttributeType("", "Value2") )
    << ( QList<AttributeType>() ) ;
  QTest::newRow("2 empty attributes")
    << ( QList<AttributeType>()
         << AttributeType("", "Value1")
         << AttributeType("", "Value1") )
    << ( QList<AttributeType>() );
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testSetAttribute()
{
  // We may get empty name, in that case error is logged
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN();

  vtkNew<vtkMRMLModelNode> node;
  node->SetAttribute("Attribute1", "Value1");
  this->NodeAttributeTableView->setInspectedNode(node.GetPointer());

  QFETCH(bool, setOnNode);
  QFETCH(QString, attribute);
  QFETCH(QString, value);
  if (setOnNode)
    {
    node->SetAttribute(attribute.isNull() ? static_cast<const char*>(nullptr) : attribute.toUtf8().constData(),
                       value.isNull() ? static_cast<const char*>(nullptr) : value.toUtf8().constData());
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

  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testSetAttribute_data()
{
  // We may get empty name, in that case error is logged
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN();

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
    QTest::newRow("null null") << (setOnNode != 0)
      << QString() << QString() << 1 << "Value1";
    QTest::newRow("null empty") << (setOnNode != 0)
      << QString() << QString("") << 1 << "Value1";
    QTest::newRow("null valid") << (setOnNode != 0)
      << QString() << QString("Value2") << 1 << "Value1";
    QTest::newRow("empty null") << (setOnNode != 0)
      << QString("") << QString() << 1 << "Value1";
    QTest::newRow("empty empty") << (setOnNode != 0)
      << QString("") << QString("") << 1 << "Value1";
    QTest::newRow("empty valid") << (setOnNode != 0)
      << QString("") << QString("Value2") << 1 << "Value1";
    QTest::newRow("valid null") << (setOnNode != 0)
      << QString("Attribute2") << QString() << 1 << "Value1";
    QTest::newRow("valid empty") << (setOnNode != 0)
      << QString("Attribute2") << QString("") << 2 << "Value1";
    QTest::newRow("valid valid") << (setOnNode != 0)
      << QString("Attribute2") << QString("Value2") << 2 << "Value1";
    QTest::newRow("replace null") << (setOnNode != 0)
      << QString("Attribute1") << QString() << 0 << QString();
    QTest::newRow("replace empty") << (setOnNode != 0)
      << QString("Attribute1") << QString("") << 1 << QString("");
    QTest::newRow("replace valid") << (setOnNode != 0)
      << QString("Attribute1") << QString("Value2") << 1 << QString("Value2");
  }

  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testSelect()
{
  // We may get empty name, in that case error is logged
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN();

  QFETCH(bool, null);
  QFETCH(QList<int>, rangeToSelect);
  QFETCH(int, expectedSelectedCellCount);

  if (!null)
    {
    QFETCH(QList<AttributeType>, attributes);
    vtkNew<vtkMRMLModelNode> node;

    foreach(const AttributeType& attribute, attributes)
      {
      node->SetAttribute(attribute.first.toUtf8(), attribute.second.toUtf8());
      }

    this->NodeAttributeTableView->setInspectedNode(node.GetPointer());
    }
  else
    {
    this->NodeAttributeTableView->setInspectedNode(nullptr);
    }

  this->NodeAttributeTableView->selectItemRange( rangeToSelect[0]
                                                ,rangeToSelect[1]
                                                ,rangeToSelect[2]
                                                ,rangeToSelect[3] );

  QCOMPARE(this->NodeAttributeTableView->selectionModel()->selectedIndexes().count(), expectedSelectedCellCount);

  if (expectedSelectedCellCount > 0)
    {
    QCOMPARE(this->NodeAttributeTableView->selectionModel()->selection().at(0).top(), rangeToSelect[0]);
    QCOMPARE(this->NodeAttributeTableView->selectionModel()->selection().at(0).left(), rangeToSelect[1]);
    QCOMPARE(this->NodeAttributeTableView->selectionModel()->selection().at(0).bottom(), rangeToSelect[2]);
    QCOMPARE(this->NodeAttributeTableView->selectionModel()->selection().at(0).right(), rangeToSelect[3]);
    }

  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testSelect_data()
{
  QTest::addColumn<bool>("null");
  QTest::addColumn<QList<AttributeType> >("attributes");
  QTest::addColumn<QList<int> >("rangeToSelect"); // top, left, bottom, right
  QTest::addColumn<int>("expectedSelectedCellCount");

  QTest::newRow("null")
    << true << QList<AttributeType>()
    << ( QList<int>() << 1 << 0 << 1 << 0 )
    << 0;
  QTest::newRow("valid with 1 cell selected")
    << false << ( QList<AttributeType>()
                  << AttributeType("Attribute1", "Value1")
                  << AttributeType("Attribute2", "Value2") )
    << ( QList<int>() << 1 << 0 << 1 << 0 )
    << 1;
  QTest::newRow("valid with 2 cells selected")
    << false << ( QList<AttributeType>()
                  << AttributeType("Attribute1", "Value1")
                  << AttributeType("Attribute2", "Value2") )
    << ( QList<int>() << 1 << 0 << 1 << 1 )
    << 2;
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testAdd()
{
  // We may get empty name, in that case error is logged
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN();

  vtkNew<vtkMRMLModelNode> node;

  QFETCH(QList<AttributeType>, attributes);
  foreach(const AttributeType& attribute, attributes)
    {
    node->SetAttribute(attribute.first.toUtf8(), attribute.second.toUtf8());
    }

  this->NodeAttributeTableView->setInspectedNode(node.GetPointer());

  this->NodeAttributeTableView->addAttribute();

  QFETCH(QList<AttributeType>, expectedAttributes);

  QStringList resultAttributes = this->NodeAttributeTableView->attributes();
  int i = 0;
  foreach (QString attribute, resultAttributes)
    {
    QCOMPARE(attribute, expectedAttributes[i].first);
    QCOMPARE(this->NodeAttributeTableView->attributeValue(attribute),
             expectedAttributes[i].second);
    ++i;
    }
  
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testAdd_data()
{
  QTest::addColumn<QList<AttributeType> >("attributes");
  QTest::addColumn<QList<AttributeType> >("expectedAttributes");

  QTest::newRow("empty attribute")
    << ( QList<AttributeType>() << AttributeType("", "") )
    << ( QList<AttributeType>() << AttributeType("NewAttributeName", "") );
  QTest::newRow("attribute")
    << ( QList<AttributeType>() << AttributeType("Attribute1", "Value1") )
    << ( QList<AttributeType>() << AttributeType("Attribute1", "Value1")
         << AttributeType("NewAttributeName", "") );
  QTest::newRow("NewAttributeName attribute")
    << ( QList<AttributeType>() << AttributeType("NewAttributeName", "Value1") )
    << ( QList<AttributeType>() << AttributeType("NewAttributeName", "Value1")
         << AttributeType("NewAttributeName1", ""));
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testRename()
{
  // We may get empty name, in that case error is logged
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN();

  vtkNew<vtkMRMLModelNode> node;
  node->SetAttribute("Attribute1", "Value1");
  node->SetAttribute("Attribute2", "Value2");
  this->NodeAttributeTableView->setInspectedNode(node.GetPointer());

  QFETCH(QString, attributeToRename);
  QFETCH(QString, newAttributeName);
  QFETCH(int, renamedAttributeRowIndex);

  // Make the new attribute name the current so that the view can store its name
  // This way in case of duplicated names, the original name is restored (default behavior)
  this->NodeAttributeTableView->selectItemRange(
    renamedAttributeRowIndex,0,renamedAttributeRowIndex,0);
  QModelIndex index(this->NodeAttributeTableView->selectionModel()->selectedIndexes().at(0));
  this->NodeAttributeTableView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);

  this->NodeAttributeTableView->renameAttribute(attributeToRename, newAttributeName);

  QFETCH(QStringList, expectedAttributes);
  QCOMPARE(this->NodeAttributeTableView->attributes(),
           expectedAttributes);

  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testRename_data()
{
  QTest::addColumn<QString>("attributeToRename");
  QTest::addColumn<QString>("newAttributeName");
  QTest::addColumn<int>("renamedAttributeRowIndex");
  QTest::addColumn<QStringList>("expectedAttributes");

  QTest::newRow("Attribute1 -> Attribute3")
    << "Attribute1" << "Attribute3" << 0
    << (QStringList() << "Attribute2" << "Attribute3");
  QTest::newRow("Attribute1 -> Attribute2")
    << "Attribute1" << "Attribute2" << 0
    << (QStringList() << "Attribute1" << "Attribute2");
  QTest::newRow("Attribute2 -> Attribute3")
    << "Attribute2" << "Attribute3" << 1
    << (QStringList() << "Attribute1" << "Attribute3");
  QTest::newRow("Attribute1 -> \"\"")
    << "Attribute1" << "" << 0
    << (QStringList() << "Attribute2");
  QTest::newRow("\"\" -> Attribute1")
    << "" << "Attribute1" << 0
    << (QStringList() << "Attribute1" << "Attribute2");
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testRemove()
{
  // We may get empty name, in that case error is logged
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN();

  QFETCH(QList<AttributeType>, attributesToAdd);
  QFETCH(QList<int>, rangeToSelect);
  QFETCH(int, expectedAttributeCountAfterRemove);
  QFETCH(QList<AttributeEmptyType>, expectedExistingAttributesAfterRemove);

  vtkNew<vtkMRMLModelNode> node;

  foreach(const AttributeType& attribute, attributesToAdd)
    {
    node->SetAttribute(attribute.first.toUtf8(), attribute.second.toUtf8());
    }

  this->NodeAttributeTableView->setInspectedNode(node.GetPointer());

  this->NodeAttributeTableView->selectItemRange(rangeToSelect.at(0),
                                                rangeToSelect.at(1),
                                                rangeToSelect.at(2),
                                                rangeToSelect.at(3));

  this->NodeAttributeTableView->removeSelectedAttributes();
  QCOMPARE(this->NodeAttributeTableView->attributeCount(), expectedAttributeCountAfterRemove);

  foreach(const AttributeEmptyType& attributeExist, expectedExistingAttributesAfterRemove)
    {
    bool isEmpty = attributeExist.second;
    QCOMPARE(this->NodeAttributeTableView->attributeValue(attributeExist.first).isEmpty(), isEmpty);

    const char * inspectedNodeAttributeValue
        = this->NodeAttributeTableView->inspectedNode()->GetAttribute(attributeExist.first.toUtf8());
    if (isEmpty)
      {
      QVERIFY(inspectedNodeAttributeValue == nullptr);
      }
    else
      {
      QVERIFY(inspectedNodeAttributeValue != nullptr);
      }
    }
  
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testRemove_data()
{
  QTest::addColumn<QList<AttributeType> >("attributesToAdd");
  QTest::addColumn<QList<int> >("rangeToSelect"); // top, left, bottom, right
  QTest::addColumn<int>("expectedAttributeCountAfterRemove");
  QTest::addColumn<QList<AttributeEmptyType> >("expectedExistingAttributesAfterRemove");

  {
  QTest::newRow("0") << ( QList<AttributeType>()
                          << AttributeType("Attribute1", "Value1")
                          << AttributeType("Attribute2", "Value2")
                          << AttributeType("Attribute3", "Value3"))
                     << ( QList<int>() << 1 << 0 << 1 << 1)
                     << 2
                     << (QList<AttributeEmptyType>()
                         << AttributeEmptyType("Attribute1", false)
                         << AttributeEmptyType("Attribute2", true)
                         << AttributeEmptyType("Attribute3", false));
  }
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testSelectAndAdd()
{
  // We may get empty name, in that case error is logged
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN();

  QFETCH(QList<AttributeType>, attributes);
  vtkNew<vtkMRMLModelNode> node;

  foreach(const AttributeType& attribute, attributes)
    {
    node->SetAttribute(attribute.first.toUtf8(), attribute.second.toUtf8());
    }

  this->NodeAttributeTableView->setInspectedNode(node.GetPointer());

  QFETCH(QList<int>, rangeToSelect);
  this->NodeAttributeTableView->selectItemRange( rangeToSelect[0]
                                                ,rangeToSelect[1]
                                                ,rangeToSelect[2]
                                                ,rangeToSelect[3] );
  QModelIndex index(this->NodeAttributeTableView->selectionModel()->selectedIndexes().at(0));
  this->NodeAttributeTableView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);

  this->NodeAttributeTableView->addAttribute();

  QFETCH(QList<AttributeType>, expectedAttributes);
  QStringList resultAttributes = this->NodeAttributeTableView->attributes();
  int i = 0;
  foreach (QString attribute, resultAttributes)
    {
    QCOMPARE(attribute, expectedAttributes[i].first);
    QCOMPARE(this->NodeAttributeTableView->attributeValue(attribute),
             expectedAttributes[i].second);
    ++i;
    }

  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewTester::testSelectAndAdd_data()
{
  QTest::addColumn<QList<AttributeType> >("attributes");
  QTest::addColumn<QList<int> >("rangeToSelect"); // top, left, bottom, right
  QTest::addColumn<QList<AttributeType> >("expectedAttributes");

  QTest::newRow("attribute name selected")
    << ( QList<AttributeType>()
                  << AttributeType("Attribute1", "Value1")
                  << AttributeType("Attribute2", "Value2") )
    << ( QList<int>() << 1 << 0 << 1 << 0 )
    << ( QList<AttributeType>()
                  << AttributeType("Attribute1", "Value1")
                  << AttributeType("Attribute2", "Value2")
                  << AttributeType("NewAttributeName", "") );
  QTest::newRow("attribute value selected")
    << ( QList<AttributeType>()
                  << AttributeType("Attribute1", "Value1")
                  << AttributeType("Attribute2", "Value2") )
    << ( QList<int>() << 1 << 1 << 1 << 1 )
    << ( QList<AttributeType>()
                  << AttributeType("Attribute1", "Value1")
                  << AttributeType("Attribute2", "Value2")
                  << AttributeType("NewAttributeName", "") );
  QTest::newRow("multiple selected")
    << ( QList<AttributeType>()
                  << AttributeType("Attribute1", "Value1")
                  << AttributeType("Attribute2", "Value2") )
    << ( QList<int>() << 0 << 0 << 1 << 1 )
    << ( QList<AttributeType>()
                  << AttributeType("Attribute1", "Value1")
                  << AttributeType("Attribute2", "Value2")
                  << AttributeType("NewAttributeName", "") );
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLNodeAttributeTableViewTest)
#include "moc_qMRMLNodeAttributeTableViewTest.cxx"
