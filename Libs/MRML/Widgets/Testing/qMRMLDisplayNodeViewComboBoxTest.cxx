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
#include "qMRMLDisplayNodeViewComboBox.h"
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkNew.h>

// ----------------------------------------------------------------------------
class qMRMLDisplayNodeViewComboBoxTester: public QObject
{
  Q_OBJECT
  qMRMLDisplayNodeViewComboBox* ComboBox;
  vtkMRMLNode* node(int i = 0);

private slots:
  void init();
  void cleanup();

  void testDefaults();
  void testSetScene();
  void testUncheck();
  void testObserveNode();
  void testAddNode();
  void testResetScene();
};

// ----------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBoxTester::init()
{
  this->ComboBox = new qMRMLDisplayNodeViewComboBox;
  vtkMRMLScene* scene = vtkMRMLScene::New();
  this->ComboBox->setMRMLScene(scene);
  vtkNew<vtkMRMLViewNode> node;
  scene->AddNode(node.GetPointer());
  vtkNew<vtkMRMLViewNode> node2;
  scene->AddNode(node2.GetPointer());
  vtkNew<vtkMRMLSliceNode> sliceNode;
  scene->AddNode(sliceNode.GetPointer());
  vtkNew<vtkMRMLModelDisplayNode> displayNode;
  scene->AddNode(displayNode.GetPointer());
  this->ComboBox->setMRMLDisplayNode(displayNode.GetPointer());

  this->ComboBox->show();
}

// ----------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBoxTester::cleanup()
{
  if (this->ComboBox == nullptr)
    {
    return;
    }
  if (this->ComboBox->mrmlScene())
    {
    this->ComboBox->mrmlScene()->Delete();
    }
  delete this->ComboBox;
  this->ComboBox = nullptr;
}

// ----------------------------------------------------------------------------
vtkMRMLNode* qMRMLDisplayNodeViewComboBoxTester::node(int index)
{
  if (this->ComboBox == nullptr)
    {
    return nullptr;
    }
  vtkMRMLScene* scene = this->ComboBox->mrmlScene();
  return scene->GetNthNodeByClass(index, "vtkMRMLAbstractViewNode");
}

// ----------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBoxTester::testDefaults()
{
  qMRMLDisplayNodeViewComboBox comboBox;
  QCOMPARE(comboBox.isEnabled(), false);
  QCOMPARE(comboBox.checkedViewNodes().count(), 0);
  QCOMPARE(comboBox.allChecked(), true);
  QCOMPARE(comboBox.noneChecked(), true);
  QCOMPARE(comboBox.checkState(nullptr), Qt::Unchecked);
}

// ----------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBoxTester::testSetScene()
{
  QCOMPARE(this->ComboBox->checkedViewNodes().count(), 3);
  QCOMPARE(this->ComboBox->allChecked(), true);
  QCOMPARE(this->ComboBox->noneChecked(), false);
  QCOMPARE(this->ComboBox->checkState(this->node(0)), Qt::Checked);
  QCOMPARE(this->ComboBox->checkState(this->node(1)), Qt::Checked);
  QCOMPARE(this->ComboBox->checkState(this->node(2)), Qt::Checked);
}

// ----------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBoxTester::testUncheck()
{
  this->ComboBox->uncheck(this->node());
  QCOMPARE(this->ComboBox->allChecked(), false);
  QCOMPARE(this->ComboBox->noneChecked(), false);
  QCOMPARE(this->ComboBox->checkedViewNodes().count(), 2);
  if (this->ComboBox->checkedViewNodes().count())
    {
    QCOMPARE(this->ComboBox->checkedViewNodes()[0], this->node(1));
    QCOMPARE(this->ComboBox->checkedViewNodes()[1], this->node(2));
    }
  QCOMPARE(this->ComboBox->uncheckedViewNodes().count(), 1);
  if (this->ComboBox->uncheckedViewNodes().count())
    {
    QCOMPARE(this->ComboBox->uncheckedViewNodes()[0], this->node(0));
    }
}

// ----------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBoxTester::testObserveNode()
{
  vtkMRMLDisplayNode* displayNode = this->ComboBox->mrmlDisplayNode();
  displayNode->AddViewNodeID(this->node(0)->GetID());
  QCOMPARE(this->ComboBox->allChecked(), false);
  QCOMPARE(this->ComboBox->noneChecked(), false);
  QCOMPARE(this->ComboBox->checkedViewNodes().count(), 1);
  if (this->ComboBox->checkedViewNodes().count())
    {
    QCOMPARE(this->ComboBox->checkedViewNodes()[0], this->node(0));
    }
  QCOMPARE(this->ComboBox->uncheckedViewNodes().count(), 2);
  if (this->ComboBox->uncheckedViewNodes().count())
    {
    QCOMPARE(this->ComboBox->uncheckedViewNodes()[0], this->node(1));
    QCOMPARE(this->ComboBox->uncheckedViewNodes()[1], this->node(2));
    }
}

// ----------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBoxTester::testAddNode()
{
  vtkMRMLScene* scene = this->ComboBox->mrmlScene();
  vtkNew<vtkMRMLViewNode> viewNode;
  scene->AddNode(viewNode.GetPointer());
  QCOMPARE(this->ComboBox->allChecked(), true);
  QCOMPARE(this->ComboBox->noneChecked(), false);
  QCOMPARE(this->ComboBox->checkedViewNodes().count(), 4);
  for (int i = 0; i < this->ComboBox->checkedViewNodes().count(); ++i)
    {
    QCOMPARE(this->ComboBox->checkedViewNodes()[i], this->node(i));
    }
  QCOMPARE(this->ComboBox->uncheckedViewNodes().count(), 0);
}

// ----------------------------------------------------------------------------
void qMRMLDisplayNodeViewComboBoxTester::testResetScene()
{
  vtkMRMLScene* scene = this->ComboBox->mrmlScene();
  this->ComboBox->setMRMLScene(nullptr);
  QCOMPARE(this->ComboBox->checkedViewNodes().count(), 0);
  scene->Delete();
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLDisplayNodeViewComboBoxTest)
#include "moc_qMRMLDisplayNodeViewComboBoxTest.cxx"
