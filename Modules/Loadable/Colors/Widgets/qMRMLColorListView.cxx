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

  This file was developed by Julien Finet, Kitware Inc. and Csaba Pinter, EBATINCA, S.L.
  and was partially funded by NIH grant 3P41RR013218-12S1 and
  by Murat Maga (Seattle Children’s Research Institute).

==============================================================================*/

// Qt includes
#include <QSortFilterProxyModel>

// qMRML includes
#include "qMRMLColorListView.h"
#include "qMRMLColorModel.h"
#include "qMRMLSortFilterColorProxyModel.h"

// MRML includes
#include <vtkMRMLColorNode.h>

//------------------------------------------------------------------------------
class qMRMLColorListViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLColorListView);

protected:
  qMRMLColorListView* const q_ptr;

public:
  qMRMLColorListViewPrivate(qMRMLColorListView& object);
  void init();
};

//------------------------------------------------------------------------------
qMRMLColorListViewPrivate::qMRMLColorListViewPrivate(qMRMLColorListView& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qMRMLColorListViewPrivate::init()
{
  Q_Q(qMRMLColorListView);

  qMRMLColorModel* colorModel = new qMRMLColorModel(q);
  colorModel->setLabelColumn(0);
  qMRMLSortFilterColorProxyModel* sortFilterModel = new qMRMLSortFilterColorProxyModel(q);
  sortFilterModel->setSourceModel(colorModel);
  q->setModel(sortFilterModel);

  q->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

//------------------------------------------------------------------------------
qMRMLColorListView::qMRMLColorListView(QWidget* _parent)
  : QListView(_parent)
  , d_ptr(new qMRMLColorListViewPrivate(*this))
{
  Q_D(qMRMLColorListView);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLColorListView::~qMRMLColorListView() = default;

//------------------------------------------------------------------------------
qMRMLColorModel* qMRMLColorListView::colorModel() const
{
  return qobject_cast<qMRMLColorModel*>(this->sortFilterProxyModel()->sourceModel());
}

//------------------------------------------------------------------------------
qMRMLSortFilterColorProxyModel* qMRMLColorListView::sortFilterProxyModel() const
{
  return qobject_cast<qMRMLSortFilterColorProxyModel*>(this->model());
}

//------------------------------------------------------------------------------
void qMRMLColorListView::setMRMLColorNode(vtkMRMLNode* node)
{
  this->setMRMLColorNode(vtkMRMLColorNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLColorListView::setMRMLColorNode(vtkMRMLColorNode* node)
{
  qMRMLColorModel* mrmlModel = this->colorModel();
  Q_ASSERT(mrmlModel);
  mrmlModel->setMRMLColorNode(node);
  this->sortFilterProxyModel()->invalidate();
  this->setCurrentIndex(this->model()->index(-1, -1));
}

//------------------------------------------------------------------------------
vtkMRMLColorNode* qMRMLColorListView::mrmlColorNode() const
{
  qMRMLColorModel* mrmlModel = this->colorModel();
  Q_ASSERT(mrmlModel);
  return mrmlModel->mrmlColorNode();
}

//------------------------------------------------------------------------------
void qMRMLColorListView::setShowOnlyNamedColors(bool enable)
{
  this->sortFilterProxyModel()->setShowEmptyColors(!enable);
  emit this->showOnlyNamedColorsChanged(enable);
}

//------------------------------------------------------------------------------
bool qMRMLColorListView::showOnlyNamedColors() const
{
  return this->sortFilterProxyModel()->filterRegExp().isEmpty();
}

//------------------------------------------------------------------------------
void qMRMLColorListView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
  if (current.isValid())
  {
    QModelIndex colorIndex = this->sortFilterProxyModel()->mapToSource(current);
    int colorEntry = this->colorModel()->colorFromIndex(colorIndex);
    emit this->colorSelected(colorEntry);
    QColor color = this->colorModel()->qcolorFromColor(colorEntry);
    emit this->colorSelected(color);
    QString name = this->colorModel()->nameFromColor(colorEntry);
    emit this->colorSelected(name);
  }
  this->QListView::currentChanged(current, previous);
}
