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
#include <QDebug>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QSortFilterProxyModel>

// CTK includes
#include <ctkColorDialog.h>

// qMRML includes
#include "qMRMLColorTableView.h"
#include "qMRMLColorModel.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLColorTableNode.h>

//------------------------------------------------------------------------------
OpacityDelegate::OpacityDelegate(QObject *parent)
  : QItemDelegate(parent)
{
}

//------------------------------------------------------------------------------
QWidget *OpacityDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
  Q_UNUSED(option);
  Q_UNUSED(index);
  QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
  editor->setSingleStep(0.1);
  editor->setMinimum(0.);
  editor->setMaximum(1.);
  return editor;
}

//------------------------------------------------------------------------------
void OpacityDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
  double value = index.model()->data(index, Qt::EditRole).toDouble();

  QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(editor);
  spinBox->setValue(value);
}

//------------------------------------------------------------------------------
void OpacityDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
  QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(editor);
  spinBox->interpretText();
  double value = spinBox->value();
  model->setData(index, value, Qt::EditRole);
}

//------------------------------------------------------------------------------
void OpacityDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
  Q_UNUSED(index);
  editor->setGeometry(option.rect);
}

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
  sortFilterModel->setFilterKeyColumn(qMRMLColorModel::LabelColumn);
  sortFilterModel->setSourceModel(colorModel);
  q->setModel(sortFilterModel);

  q->setSelectionBehavior(QAbstractItemView::SelectRows);
  q->horizontalHeader()->setResizeMode(qMRMLColorModel::ColorColumn, QHeaderView::ResizeToContents);
  q->horizontalHeader()->setResizeMode(qMRMLColorModel::LabelColumn, QHeaderView::Stretch);
  q->horizontalHeader()->setResizeMode(qMRMLColorModel::OpacityColumn, QHeaderView::ResizeToContents);

  OpacityDelegate* delegate = new OpacityDelegate;
  q->setItemDelegateForColumn(qMRMLColorModel::OpacityColumn, delegate);
  
  QObject::connect(q, SIGNAL(doubleClicked(const QModelIndex& )),
                   q, SLOT(onDoubleClicked(const QModelIndex&)));
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
  
  this->horizontalHeader()->setResizeMode(qMRMLColorModel::ColorColumn, QHeaderView::Fixed);
  this->horizontalHeader()->setResizeMode(qMRMLColorModel::LabelColumn, QHeaderView::Fixed);
  this->horizontalHeader()->setResizeMode(qMRMLColorModel::OpacityColumn, QHeaderView::Fixed);

  mrmlModel->setMRMLColorNode(node);
  this->sortFilterProxyModel()->invalidate();

  this->horizontalHeader()->setResizeMode(qMRMLColorModel::ColorColumn, QHeaderView::ResizeToContents);
  this->horizontalHeader()->setResizeMode(qMRMLColorModel::LabelColumn, QHeaderView::Stretch);
  this->horizontalHeader()->setResizeMode(qMRMLColorModel::OpacityColumn, QHeaderView::ResizeToContents);

  this->setEditTriggers( (node && node->GetType() == vtkMRMLColorTableNode::User) ?
      QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed :
      QAbstractItemView::NoEditTriggers);
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
    this->sortFilterProxyModel()->setFilterRegExp(QRegExp("^(?!\\(none\\))"));
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

//------------------------------------------------------------------------------
void qMRMLColorTableView::onDoubleClicked(const QModelIndex& index)
{
  if (index.column() != qMRMLColorModel::ColorColumn ||
      this->editTriggers() == 0)
    {
    return;
    }
  QColor color = this->sortFilterProxyModel()->data(index, qMRMLColorModel::ColorRole).value<QColor>();
  QColor newColor = ctkColorDialog::getColor(color, this, "Select a new color", 0);
  if (!newColor.isValid())
    {
    return;
    }
  this->sortFilterProxyModel()->setData(index, newColor, qMRMLColorModel::ColorRole);
}
