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

// QT includes
#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QDebug>
#include <QSpinBox>
#include <QAction>

// MRML includes
#include "vtkMRMLScene.h"

// MRML Widgets includes
#include "qMRMLNodeComboBox.h"

// Subject hierarchy widgets includes
#include "qMRMLSceneSubjectHierarchyModel.h"
#include "qMRMLTransformItemDelegate.h"

//------------------------------------------------------------------------------
qMRMLTransformItemDelegate::qMRMLTransformItemDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
  this->MRMLScene = NULL;
  this->FixedRowHeight = -1;

  this->RemoveTransformAction = new QAction("Remove transforms from branch", this);
  connect(this->RemoveTransformAction, SIGNAL(triggered()), this, SIGNAL(removeTransformsFromBranchOfCurrentNode()));

  this->HardenAction = new QAction("Harden transform on branch", this);
  connect(this->HardenAction, SIGNAL(triggered()), this, SIGNAL(hardenTransformOnBranchOfCurrentNode()));
}

//------------------------------------------------------------------------------
qMRMLTransformItemDelegate::~qMRMLTransformItemDelegate()
{
}

//------------------------------------------------------------------------------
void qMRMLTransformItemDelegate::setMRMLScene(vtkMRMLScene* scene)
{
  if (scene)
    {
    this->MRMLScene = scene;
    }
}

//------------------------------------------------------------------------------
bool qMRMLTransformItemDelegate::isTransform(const QModelIndex& index)const
{
  QVariant whatsThisData = index.data(Qt::WhatsThisRole);
  if (whatsThisData.toString() == "Transform")
    {
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
QWidget *qMRMLTransformItemDelegate
::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
               const QModelIndex &index) const
{
  if (this->isTransform(index))
    {
    qMRMLNodeComboBox* transformNodeCombobox = new qMRMLNodeComboBox(parent);
    QStringList nodeTypes;
    nodeTypes << QString("vtkMRMLTransformNode");
    transformNodeCombobox->setMaximumHeight(16);
    transformNodeCombobox->setNodeTypes(nodeTypes);
    transformNodeCombobox->setNoneEnabled(true);
    transformNodeCombobox->setRemoveEnabled(false);
    transformNodeCombobox->setRenameEnabled(false);
    transformNodeCombobox->setAddEnabled(false);
    transformNodeCombobox->addMenuAction(this->RemoveTransformAction);
    transformNodeCombobox->addMenuAction(this->HardenAction);
    transformNodeCombobox->setMRMLScene(this->MRMLScene);
    connect(transformNodeCombobox, SIGNAL(currentNodeIDChanged(QString)),
            this, SLOT(commitAndClose()), Qt::QueuedConnection);
    return transformNodeCombobox;
    }
  return this->QStyledItemDelegate::createEditor(parent, option, index);
}

//------------------------------------------------------------------------------
void qMRMLTransformItemDelegate::setEditorData(QWidget *editor,
                                      const QModelIndex &index) const
{
  if (this->isTransform(index))
    {
    QString transformNodeID = index.data(qMRMLSceneSubjectHierarchyModel::TransformIDRole).toString();
    qMRMLNodeComboBox* transformNodeCombobox = qobject_cast<qMRMLNodeComboBox*>(editor);
    if (!transformNodeCombobox)
      {
      qCritical() << "qMRMLTransformItemDelegate::setEditorData: Invalid editor widget!";
      return;
      }
    transformNodeCombobox->blockSignals(true);
    transformNodeCombobox->setCurrentNodeID(transformNodeID);
    transformNodeCombobox->blockSignals(false);
    }
  else
    {
    this->QStyledItemDelegate::setEditorData(editor, index);
    }
}

//------------------------------------------------------------------------------
void qMRMLTransformItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
  if (this->isTransform(index))
    {
    qMRMLNodeComboBox* transformNodeCombobox = qobject_cast<qMRMLNodeComboBox*>(editor);
    if (!transformNodeCombobox)
      {
      qCritical() << "qMRMLTransformItemDelegate::setModelData: Invalid editor widget!";
      return;
      }
    QString transformNodeID = transformNodeCombobox->currentNodeID();
    model->setData(index, transformNodeID, qMRMLSceneSubjectHierarchyModel::TransformIDRole);
    }
  else
    {
    this->QStyledItemDelegate::setModelData(editor, model, index);
    }
}

//------------------------------------------------------------------------------
void qMRMLTransformItemDelegate::commitAndClose()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  emit commitData(editor);
  emit closeEditor(editor);
}

//------------------------------------------------------------------------------
QSize qMRMLTransformItemDelegate
::sizeHint(const QStyleOptionViewItem &option,
           const QModelIndex &index) const
{
  // Get default size hint
  QSize resultSizeHint = this->QStyledItemDelegate::sizeHint(option, index);
  if (this->FixedRowHeight > 0)
    {
    resultSizeHint.setHeight(this->FixedRowHeight);
    }
  if (this->isTransform(index))
    {
    // Optionally can return the size hint of a dummy widget for the transforms
    return resultSizeHint;
    }
  return resultSizeHint;
}

//------------------------------------------------------------------------------
void qMRMLTransformItemDelegate
::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const
{
  if (this->isTransform(index))
    {
    editor->setGeometry(option.rect);
    }
  else
    {
    this->QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    }
}

//------------------------------------------------------------------------------
bool qMRMLTransformItemDelegate::eventFilter(QObject *object, QEvent *event)
{
  qMRMLNodeComboBox* editor = qobject_cast<qMRMLNodeComboBox*>(object);
  if (editor &&
      (event->type() == QEvent::FocusOut ||
      (event->type() == QEvent::Hide && editor->isWindow())))
    {
    // The Hide event will take care of he editors that are in fact complete dialogs
    if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor))
      {
      QWidget* widget = QApplication::focusWidget();
      while (widget)
        {
        qMRMLNodeComboBox* transformNodeCombobox = qobject_cast<qMRMLNodeComboBox*>(editor);
        if (transformNodeCombobox && transformNodeCombobox == editor)
          {
          return false;
          }
        widget = widget->parentWidget();
        }
      }
    }
  return this->QStyledItemDelegate::eventFilter(object, event);
}

//------------------------------------------------------------------------------
void qMRMLTransformItemDelegate::setFixedRowHeight(int height)
{
  this->FixedRowHeight = height;
}
