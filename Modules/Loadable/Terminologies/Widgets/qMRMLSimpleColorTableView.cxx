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

  This file was originally developed by Csaba Pinter, EBATINCA, S.L.
  and was funded by by Murat Maga (Seattle Children's Research Institute).

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QDebug>
#include <QHeaderView>

// CTK includes
#include <ctkColorDialog.h>

// qMRML includes
#include "qMRMLColorModel.h"
#include "qMRMLItemDelegate.h"
#include "qMRMLSimpleColorTableView.h"
#include "qMRMLSortFilterColorProxyModel.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>

//------------------------------------------------------------------------------
class qMRMLSimpleColorTableViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSimpleColorTableView);
protected:
  qMRMLSimpleColorTableView* const q_ptr;
public:
  qMRMLSimpleColorTableViewPrivate(qMRMLSimpleColorTableView& object);
  void init();
};

//------------------------------------------------------------------------------
qMRMLSimpleColorTableViewPrivate::qMRMLSimpleColorTableViewPrivate(qMRMLSimpleColorTableView& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qMRMLSimpleColorTableViewPrivate::init()
{
  Q_Q(qMRMLSimpleColorTableView);

  q->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

  qMRMLColorModel* colorModel = new qMRMLColorModel(q);
  qMRMLSortFilterColorProxyModel* sortFilterModel = new qMRMLSortFilterColorProxyModel(q);
  sortFilterModel->setSourceModel(colorModel);
  q->setModel(sortFilterModel);

  q->setSelectionBehavior(QAbstractItemView::SelectRows);
  q->horizontalHeader()->setStretchLastSection(true);
  q->horizontalHeader()->setSectionResizeMode(colorModel->colorColumn(), QHeaderView::ResizeToContents);
  q->horizontalHeader()->setSectionResizeMode(colorModel->labelColumn(), QHeaderView::ResizeToContents);

  q->setColumnHidden(colorModel->opacityColumn(), true);
}

//------------------------------------------------------------------------------
qMRMLSimpleColorTableView::qMRMLSimpleColorTableView(QWidget* _parent)
  : QTableView(_parent)
  , d_ptr(new qMRMLSimpleColorTableViewPrivate(*this))
{
  Q_D(qMRMLSimpleColorTableView);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLSimpleColorTableView::~qMRMLSimpleColorTableView() = default;

//------------------------------------------------------------------------------
qMRMLColorModel* qMRMLSimpleColorTableView::colorModel()const
{
  return qobject_cast<qMRMLColorModel*>(this->sortFilterProxyModel()->sourceModel());
}

//------------------------------------------------------------------------------
qMRMLSortFilterColorProxyModel* qMRMLSimpleColorTableView::sortFilterProxyModel()const
{
  return qobject_cast<qMRMLSortFilterColorProxyModel*>(this->model());
}

//------------------------------------------------------------------------------
void qMRMLSimpleColorTableView::setMRMLColorNode(vtkMRMLNode* node)
{
  this->setMRMLColorNode(vtkMRMLColorNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLSimpleColorTableView::setMRMLColorNode(vtkMRMLColorNode* node)
{
  qMRMLColorModel* mrmlModel = this->colorModel();
  Q_ASSERT(mrmlModel);

  mrmlModel->setMRMLColorNode(node);
  this->sortFilterProxyModel()->invalidate();

  this->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

//------------------------------------------------------------------------------
vtkMRMLColorNode* qMRMLSimpleColorTableView::mrmlColorNode()const
{
  qMRMLColorModel* mrmlModel = this->colorModel();
  Q_ASSERT(mrmlModel);
  return mrmlModel->mrmlColorNode();
}

//------------------------------------------------------------------------------
bool qMRMLSimpleColorTableView::selectColorByIndex(int colorIndex)
{
  QSortFilterProxyModel* sortFilterModel = this->sortFilterProxyModel();
  qMRMLColorModel* colorModel = this->colorModel();
  vtkMRMLColorNode* colorNode = this->mrmlColorNode();
  if (colorNode == nullptr)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid color node in table view";
    return false;
  }

  QModelIndexList foundIndices = colorModel->match(colorModel->index(0,0), qMRMLItemDelegate::ColorEntryRole,
    colorIndex, 1, Qt::MatchExactly | Qt::MatchRecursive);
  if (foundIndices.size() == 0)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to find color model index by color index " << colorIndex
      << " in color node " << colorNode->GetName();
    return false;
  }

  QModelIndex foundIndex = sortFilterModel->mapFromSource(foundIndices[0]);
  this->selectionModel()->select(foundIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);

  // set current index to the found index, but in the label column
  QModelIndex foundLabelIndex = foundIndex.sibling(foundIndex.row(), colorModel->labelColumn());
  this->selectionModel()->setCurrentIndex(foundLabelIndex, QItemSelectionModel::Current);

  // Scroll to the item to make it visible
  this->scrollTo(foundIndex);
  return true;
}

//------------------------------------------------------------------------------
int qMRMLSimpleColorTableView::selectedColorIndex()const
{
  QSortFilterProxyModel* sortFilterModel = this->sortFilterProxyModel();
  if (!sortFilterModel)
  {
    return -1;
  }
  qMRMLColorModel* colorModel = this->colorModel();
  if (!colorModel)
  {
    return -1;
  }
  int colorIndex = colorModel->colorFromIndex(sortFilterModel->mapToSource(this->currentIndex()));
  return colorIndex;
}
