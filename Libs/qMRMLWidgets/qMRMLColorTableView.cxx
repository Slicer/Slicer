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

// QT includes
#include <QHeaderView>
#include <QSortFilterProxyModel>

// qMRML includes
#include "qMRMLColorTableView.h"
#include "qMRMLColorModel.h"

// MRML includes
#include <vtkMRMLColorNode.h>

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
  QSortFilterProxyModel* sortFilterModel = new QSortFilterProxyModel(q);
  sortFilterModel->setSourceModel(colorModel);
  q->setModel(sortFilterModel);
  
  q->horizontalHeader()->setResizeMode(qMRMLColorModel::ColorColumn, QHeaderView::Stretch);
  q->horizontalHeader()->setResizeMode(qMRMLColorModel::OpacityColumn, QHeaderView::ResizeToContents);
}

//------------------------------------------------------------------------------
qMRMLColorTableView::qMRMLColorTableView(QWidget *_parent)
  : QTableView(_parent)
  , d_ptr(new qMRMLColorTableViewPrivate(*this))
{
  Q_D(qMRMLColorTableView);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLColorTableView::~qMRMLColorTableView()
{
}

//------------------------------------------------------------------------------
qMRMLColorModel* qMRMLColorTableView::colorModel()const
{
  return qobject_cast<qMRMLColorModel*>(this->sortFilterProxyModel()->sourceModel());
}

//------------------------------------------------------------------------------
QSortFilterProxyModel* qMRMLColorTableView::sortFilterProxyModel()const
{
  return qobject_cast<QSortFilterProxyModel*>(this->model());
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
}

//------------------------------------------------------------------------------
vtkMRMLColorNode* qMRMLColorTableView::mrmlColorNode()const
{
  qMRMLColorModel* mrmlModel = this->colorModel();
  Q_ASSERT(mrmlModel);
  return mrmlModel->mrmlColorNode();
}

//------------------------------------------------------------------------------
void qMRMLColorTableView::setShowOnlyNamedColors(bool enable)
{
  if (enable)
    {
    this->sortFilterProxyModel()->setFilterFixedString(QString("(none)"));
    }
  else
    {
    this->sortFilterProxyModel()->setFilterRegExp(QRegExp());
    }
}

//------------------------------------------------------------------------------
bool qMRMLColorTableView::showOnlyNamedColors()const
{
  return this->sortFilterProxyModel()->filterRegExp().isEmpty();
}
