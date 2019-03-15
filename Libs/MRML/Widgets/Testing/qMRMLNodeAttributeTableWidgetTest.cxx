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

// CTK includes
#include "ctkTest.h"

// MRML includes
#include "qMRMLNodeAttributeTableWidget.h"
#include "qMRMLNodeAttributeTableView.h"
#include <vtkMRMLModelNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// ----------------------------------------------------------------------------
class qMRMLNodeAttributeTableWidgetTester: public QObject
{
  Q_OBJECT
  qMRMLNodeAttributeTableWidget* NodeAttributeTableWidget;
  vtkMRMLNode* node(int i = 0);

private slots:
  void init();
  void cleanup();

  void testSetNode();
};


// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableWidgetTester::init()
{
  this->NodeAttributeTableWidget = new qMRMLNodeAttributeTableWidget;
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableWidgetTester::cleanup()
{
  if (this->NodeAttributeTableWidget != nullptr)
    {
    this->NodeAttributeTableWidget->setMRMLNode(nullptr);
    delete this->NodeAttributeTableWidget;
    this->NodeAttributeTableWidget = nullptr;
    }
}

// ----------------------------------------------------------------------------
void qMRMLNodeAttributeTableWidgetTester::testSetNode()
{
  vtkSmartPointer<vtkMRMLModelNode> node = vtkSmartPointer<vtkMRMLModelNode>::New();
  node->SetAttribute("Attribute1", "Value1");
  node->SetAttribute("Attribute2", "Value2");
  this->NodeAttributeTableWidget->setMRMLNode(node);
  QCOMPARE(this->NodeAttributeTableWidget->tableView()->attributeCount(), 2);

  this->NodeAttributeTableWidget->setMRMLNode(nullptr);
  QCOMPARE(this->NodeAttributeTableWidget->tableView()->attributeCount(), 0);
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLNodeAttributeTableWidgetTest)
#include "moc_qMRMLNodeAttributeTableWidgetTest.cxx"
