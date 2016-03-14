/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

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

// qMRML includes
#include "qMRMLTableItemDelegate.h"

// Qt includes
#include <QCheckBox>
#include <QDebug>
#include <QApplication>
#include <QHBoxLayout>

// MRML includes
#include <vtkMRMLTableNode.h>

//------------------------------------------------------------------------------
qMRMLTableItemDelegate::qMRMLTableItemDelegate(QObject *parent)
  : QStyledItemDelegate(parent) { }

//------------------------------------------------------------------------------
qMRMLTableItemDelegate::~qMRMLTableItemDelegate()
{
}

// --------------------------------------------------------------------------
void qMRMLTableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  int columnType = index.data(Qt::WhatsThisRole).toInt();
  if (columnType == vtkMRMLTableNode::BoolType)
    {
    QStyle* style = QApplication::style();
 
    QRect checkBoxRect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option);
    int checkboxWidth = checkBoxRect.width();
    int checkboxHeight = checkBoxRect.height();
 
    int centerX = option.rect.left() + qMax(option.rect.width()/2-checkboxWidth/2, 0);
    int centerY = option.rect.top() + qMax(option.rect.height()/2-checkboxHeight/2, 0);
    QStyleOptionViewItem modifiedOption(option);
    modifiedOption.rect.moveTo(centerX, centerY);
    modifiedOption.rect.setSize(QSize(checkboxWidth, checkboxHeight));
    modifiedOption.state |= QStyle::State_On;
 
    style->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &modifiedOption, painter);
    }
  else
    {
    QStyledItemDelegate::paint(painter, option, index);
    }
}
//------------------------------------------------------------------------------
QWidget* qMRMLTableItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  int columnType = index.data(Qt::WhatsThisRole).toInt();
  if (columnType == vtkMRMLTableNode::BoolType)
    {
    QWidget* widget = new QWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout();
    QCheckBox* checkBox = new QCheckBox(widget);
    layout->addWidget(checkBox);
    layout->setAlignment(checkBox, Qt::AlignHCenter | Qt::AlignVCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    widget->setLayout(layout);
    connect(checkBox, SIGNAL(toggled(bool)),
            this, SLOT(commitAndClose()), Qt::QueuedConnection);
    return widget;
    }

  return this->Superclass::createEditor(parent, option, index);
}

//------------------------------------------------------------------------------
void qMRMLTableItemDelegate::setEditorData(QWidget *editor,
                                      const QModelIndex &index) const
{
  int columnType = index.data(Qt::WhatsThisRole).toInt();
  if (columnType == vtkMRMLTableNode::BoolType)
    {
    int data = index.data(Qt::EditRole).toInt(); // vtkBitArray essentially returns integers
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(editor);
    if (checkBox)
      {
      checkBox->blockSignals(true);
      checkBox->setChecked(data);
      checkBox->blockSignals(false);
      }
    else
      {
      qWarning() << Q_FUNC_INFO << " failed: checkBox is invalid";
      }
    }
  else
    {
    this->Superclass::setEditorData(editor, index);
    }
}

//------------------------------------------------------------------------------
void qMRMLTableItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
  int columnType = index.data(Qt::WhatsThisRole).toInt();
  if (columnType == vtkMRMLTableNode::BoolType)
    {
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(editor);
    if (checkBox)
      {
      model->setData(index, checkBox->isChecked(), Qt::EditRole);
      }
    else
      {
      qWarning() << Q_FUNC_INFO << " failed: checkBox is invalid";
      }
    }
  else
    {
    this->Superclass::setModelData(editor, model, index);
    }
}

//------------------------------------------------------------------------------
void qMRMLTableItemDelegate::commitSenderData()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  emit commitData(editor);
}

//------------------------------------------------------------------------------
void qMRMLTableItemDelegate::commitAndClose()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  emit commitData(editor);
  emit closeEditor(editor);
}

//------------------------------------------------------------------------------
void qMRMLTableItemDelegate
::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const
{
  int columnType = index.data(Qt::WhatsThisRole).toInt();
  if (columnType == vtkMRMLTableNode::BoolType)
    {
    editor->setGeometry(option.rect);
    }
  else
    {
    this->Superclass::updateEditorGeometry(editor, option, index);
    }
}
