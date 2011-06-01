/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QAction>
#include <QDebug>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QListView>
#include <QStandardItemModel>

// CTK includes
#include <ctkComboBox.h>
#include <ctkCheckableComboBox.h>

// qMRMLWidgets includes
#include "qMRMLCheckableNodeComboBox.h"
#include "qMRMLNodeComboBox_p.h"
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterProxyModel.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// -----------------------------------------------------------------------------
class qMRMLCheckableNodeComboBoxPrivate: public qMRMLNodeComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qMRMLCheckableNodeComboBox);
protected:
  qMRMLCheckableNodeComboBox* const q_ptr;
  virtual void setModel(QAbstractItemModel* model);
public:
  qMRMLCheckableNodeComboBoxPrivate(qMRMLCheckableNodeComboBox& object);
  virtual void init(QAbstractItemModel* model);
};

// -----------------------------------------------------------------------------
qMRMLCheckableNodeComboBoxPrivate
::qMRMLCheckableNodeComboBoxPrivate(qMRMLCheckableNodeComboBox& object)
  : qMRMLNodeComboBoxPrivate(object)
  , q_ptr(&object)
{
}

// -----------------------------------------------------------------------------
void qMRMLCheckableNodeComboBoxPrivate::init(QAbstractItemModel* model)
{
  Q_Q(qMRMLCheckableNodeComboBox);
  this->ComboBox = new ctkCheckableComboBox;

  this->qMRMLNodeComboBoxPrivate::init(model);

  q->setAddEnabled(false);
  q->setRemoveEnabled(false);
  q->setEditEnabled(false);
  q->setRenameEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLCheckableNodeComboBoxPrivate::setModel(QAbstractItemModel* model)
{
  qobject_cast<ctkCheckableComboBox*>(this->ComboBox)->setCheckableModel(model);
  this->qMRMLNodeComboBoxPrivate::setModel(model);
}

// --------------------------------------------------------------------------
// qMRMLCheckableNodeComboBox

// --------------------------------------------------------------------------
qMRMLCheckableNodeComboBox::qMRMLCheckableNodeComboBox(QWidget* parentWidget)
  : Superclass(new qMRMLCheckableNodeComboBoxPrivate(*this), parentWidget)
{
}

// --------------------------------------------------------------------------
qMRMLCheckableNodeComboBox::~qMRMLCheckableNodeComboBox()
{
}
