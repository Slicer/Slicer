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
#include "qMRMLColorModel.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>

// VTK includes
#include <vtkNew.h>

// --------------------------------------------------------------------------
class qMRMLColorModelTester: public QObject
{
  Q_OBJECT
private:

  qMRMLColorModel * ColorModel;

private slots:
  void init();
  void cleanup();

  void testSetMRMLColorNode();

  void testSetNoneEnabled();
};

// ----------------------------------------------------------------------------
void qMRMLColorModelTester::init()
{
  this->ColorModel = new qMRMLColorModel();
}

// ----------------------------------------------------------------------------
void qMRMLColorModelTester::cleanup()
{
  if (this->ColorModel == nullptr)
    {
    return;
    }
  delete this->ColorModel;
}

// ----------------------------------------------------------------------------
void qMRMLColorModelTester::testSetMRMLColorNode()
{
  QVERIFY(this->ColorModel->mrmlColorNode() == nullptr);

  vtkNew<vtkMRMLColorTableNode> colorTableNode;
  this->ColorModel->setMRMLColorNode(colorTableNode.GetPointer());
  QCOMPARE(this->ColorModel->mrmlColorNode(), colorTableNode.GetPointer());

  this->ColorModel->setMRMLColorNode(nullptr);
}

// ----------------------------------------------------------------------------
void qMRMLColorModelTester::testSetNoneEnabled()
{
  QVERIFY(!this->ColorModel->noneEnabled());

  this->ColorModel->setNoneEnabled(true);
  QVERIFY(this->ColorModel->noneEnabled());

  this->ColorModel->setNoneEnabled(false);
  QVERIFY(!this->ColorModel->noneEnabled());
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLColorModelTest)
#include "moc_qMRMLColorModelTest.cxx"
