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
#include <QHeaderView>
#include <QSortFilterProxyModel>

// CTK includes
#include <ctkColorDialog.h>

// qMRML includes
#include "qMRMLColorTableView.h"
#include "qMRMLColorModel.h"
#include "qMRMLItemDelegate.h"
#include "qMRMLSortFilterColorProxyModel.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>

// Colors widgets includes
#include "qSlicerColorTableTerminologyDelegate.h"

//------------------------------------------------------------------------------
class qMRMLColorTableViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLColorTableView);

protected:
  qMRMLColorTableView* const q_ptr;

public:
  qMRMLColorTableViewPrivate(qMRMLColorTableView& object);
  void init();
};

//------------------------------------------------------------------------------
qMRMLColorTableViewPrivate::qMRMLColorTableViewPrivate(qMRMLColorTableView& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qMRMLColorTableViewPrivate::init()
{
  Q_Q(qMRMLColorTableView);

  qMRMLColorModel* colorModel = new qMRMLColorModel(q);
  qMRMLSortFilterColorProxyModel* sortFilterModel = new qMRMLSortFilterColorProxyModel(q);
  sortFilterModel->setFilterKeyColumn(colorModel->labelColumn());
  sortFilterModel->setSourceModel(colorModel);
  q->setModel(sortFilterModel);

  q->setSelectionBehavior(QAbstractItemView::SelectRows);

  // Note: resizing to contents can be extremely slow on large color tables.
  q->horizontalHeader()->setStretchLastSection(false);
  q->horizontalHeader()->setSectionResizeMode(colorModel->colorColumn(), QHeaderView::ResizeToContents);
  q->horizontalHeader()->setSectionResizeMode(colorModel->opacityColumn(), QHeaderView::ResizeToContents);
  q->horizontalHeader()->setSectionResizeMode(colorModel->labelColumn(), QHeaderView::Stretch);
  q->horizontalHeader()->setSectionResizeMode(colorModel->terminologyColumn(), QHeaderView::Stretch);

  q->setItemDelegate(new qMRMLItemDelegate(q));
  q->setItemDelegateForColumn(colorModel->terminologyColumn(), new qSlicerColorTableTerminologyDelegate(q));

  q->sortFilterProxyModel()->setShowEmptyColors(false);
}

//------------------------------------------------------------------------------
qMRMLColorTableView::qMRMLColorTableView(QWidget* _parent)
  : QTableView(_parent)
  , d_ptr(new qMRMLColorTableViewPrivate(*this))
{
  Q_D(qMRMLColorTableView);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLColorTableView::~qMRMLColorTableView() = default;

//------------------------------------------------------------------------------
qMRMLColorModel* qMRMLColorTableView::colorModel() const
{
  return qobject_cast<qMRMLColorModel*>(this->sortFilterProxyModel()->sourceModel());
}

//------------------------------------------------------------------------------
qMRMLSortFilterColorProxyModel* qMRMLColorTableView::sortFilterProxyModel() const
{
  return qobject_cast<qMRMLSortFilterColorProxyModel*>(this->model());
}

//------------------------------------------------------------------------------
void qMRMLColorTableView::setMRMLColorNode(vtkMRMLNode* node)
{
  this->setMRMLColorNode(vtkMRMLColorNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLColorTableView::setMRMLColorNode(vtkMRMLColorNode* node)
{
  qMRMLColorModel* mrmlModel = this->colorModel();
  Q_ASSERT(mrmlModel);

  mrmlModel->setMRMLColorNode(node);
  this->sortFilterProxyModel()->invalidate();

  this->setEditTriggers((node && node->GetType() == vtkMRMLColorTableNode::User) ? QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed
                                                                                 : QAbstractItemView::NoEditTriggers);
}

//------------------------------------------------------------------------------
vtkMRMLColorNode* qMRMLColorTableView::mrmlColorNode() const
{
  qMRMLColorModel* mrmlModel = this->colorModel();
  Q_ASSERT(mrmlModel);
  return mrmlModel->mrmlColorNode();
}

//------------------------------------------------------------------------------
void qMRMLColorTableView::setShowOnlyNamedColors(bool enable)
{
  this->sortFilterProxyModel()->setShowEmptyColors(!enable);
  emit this->showOnlyNamedColorsChanged(enable);
}

//------------------------------------------------------------------------------
bool qMRMLColorTableView::showOnlyNamedColors() const
{
  return this->sortFilterProxyModel()->filterRegExp().isEmpty();
}

//------------------------------------------------------------------------------
int qMRMLColorTableView::rowFromColorName(const QString& colorName) const
{
  int index = this->colorModel()->colorFromName(colorName);
  return this->rowFromColorIndex(index);
}

//------------------------------------------------------------------------------
int qMRMLColorTableView::rowFromColorIndex(int colorIndex) const
{
  QModelIndexList indexes = this->colorModel()->indexes(colorIndex);
  if (indexes.isEmpty())
  {
    return -1;
  }
  QModelIndex sortedIndex = this->sortFilterProxyModel()->mapFromSource(indexes[0]);
  return sortedIndex.row();
}
