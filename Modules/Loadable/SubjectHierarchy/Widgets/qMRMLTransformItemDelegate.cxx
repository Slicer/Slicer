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

// Qt includes
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QShowEvent>

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"

// Subject hierarchy widgets includes
#include "qMRMLSubjectHierarchyModel.h"
#include "qMRMLTransformItemDelegate.h"

//------------------------------------------------------------------------------
DelegateMenu::DelegateMenu(QWidget* parent)
  : QMenu(parent)
  , SelectedTransformNodeID(QString())
{
}

void DelegateMenu::showEvent(QShowEvent* event)
{
  Q_UNUSED(event);
  this->move(QCursor::pos());
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLTransformItemDelegate::qMRMLTransformItemDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
  this->MRMLScene = nullptr;
  this->FixedRowHeight = -1;

  this->NoneAction = new QAction("None", this);
  this->NoneAction->setCheckable(true);
  this->NoneAction->setToolTip(tr("Remove parent transform from all the nodes in this branch"));
  connect(this->NoneAction, SIGNAL(triggered()), this, SIGNAL(removeTransformsFromBranchOfCurrentItem()));

  this->TransformActionGroup = new QActionGroup(this);
  this->TransformActionGroup->addAction(this->NoneAction);

  this->HardenAction = new QAction("Harden transform", this);
  this->HardenAction->setToolTip(tr("Harden parent transforms on all the nodes in this branch"));
  connect(this->HardenAction, SIGNAL(triggered()), this, SIGNAL(hardenTransformOnBranchOfCurrentItem()));
}

//------------------------------------------------------------------------------
qMRMLTransformItemDelegate::~qMRMLTransformItemDelegate() = default;

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
QWidget *qMRMLTransformItemDelegate::createEditor(
  QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  if (this->isTransform(index))
    {
    DelegateMenu* menu = new DelegateMenu(parent->parentWidget());
    menu->setAttribute(Qt::WA_DeleteOnClose, true);

    menu->addAction(this->NoneAction);

    std::vector<vtkMRMLNode*> transformNodes;
    this->MRMLScene->GetNodesByClass("vtkMRMLTransformNode", transformNodes);
    for (std::vector<vtkMRMLNode*>::iterator it = transformNodes.begin(); it != transformNodes.end(); ++it)
      {
      vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(*it);
      if (!transformNode || transformNode->GetHideFromEditors())
        {
        continue;
        }
      QAction* nodeAction = new QAction(transformNode->GetName(), menu);
      nodeAction->setData(QString(transformNode->GetID()));
      nodeAction->setCheckable(true);
      connect(nodeAction, SIGNAL(triggered()), this, SLOT(transformActionSelected()), Qt::DirectConnection);
      menu->addAction(nodeAction);
      this->TransformActionGroup->addAction(nodeAction);
      }

    menu->addSeparator();
    menu->addAction(this->HardenAction);

    menu->setMinimumWidth(menu->sizeHint().width());
    menu->setMinimumHeight(menu->sizeHint().height());
    return menu;
    }

  return this->QStyledItemDelegate::createEditor(parent, option, index);
}

//------------------------------------------------------------------------------
void qMRMLTransformItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  if (this->isTransform(index))
    {
    QString transformNodeID = index.data(qMRMLSubjectHierarchyModel::TransformIDRole).toString();
    DelegateMenu* menu = dynamic_cast<DelegateMenu*>(editor);
    if (!menu)
      {
      qCritical() << Q_FUNC_INFO << ": Invalid editor widget";
      return;
      }
    QAction* actionForTransform = nullptr;
    if (transformNodeID.isEmpty())
      {
      actionForTransform = this->NoneAction;
      }
    else
      {
      foreach (QAction* action, menu->actions())
        {
        if (!action->data().toString().compare(transformNodeID))
          {
          actionForTransform = action;
          }
        else
          {
          action->setChecked(false);
          }
        }
      if (!actionForTransform)
        {
        qCritical() << Q_FUNC_INFO << ": Failed to find action for transform '" << transformNodeID << "'";
        return;
        }
      }
    actionForTransform->setChecked(true);

    menu->blockSignals(true);
    menu->setActiveAction(actionForTransform);
    menu->SelectedTransformNodeID = transformNodeID;
    menu->blockSignals(false);
    }
  else
    {
    this->QStyledItemDelegate::setEditorData(editor, index);
    }
}

//------------------------------------------------------------------------------
void qMRMLTransformItemDelegate::setModelData(
  QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  if (this->isTransform(index))
    {
    DelegateMenu* menu = dynamic_cast<DelegateMenu*>(editor);
    if (!menu)
      {
      qCritical() << Q_FUNC_INFO << ": Invalid editor widget";
      return;
      }
    model->setData(index, menu->SelectedTransformNodeID, qMRMLSubjectHierarchyModel::TransformIDRole);
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

  QList<QAction*> actionGroupActions = this->TransformActionGroup->actions();
  foreach (QAction* action, actionGroupActions)
    {
    if (action != this->NoneAction)
      {
      this->TransformActionGroup->removeAction(action);
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLTransformItemDelegate::transformActionSelected()
{
  QAction* action = qobject_cast<QAction*>(this->sender());
  DelegateMenu* menu = dynamic_cast<DelegateMenu*>(action->parent());
  if (!menu)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid editor widget";
    return;
    }
  menu->SelectedTransformNodeID = action->data().toString();

  emit commitData(menu);
}

//------------------------------------------------------------------------------
QSize qMRMLTransformItemDelegate::sizeHint(
  const QStyleOptionViewItem &option, const QModelIndex &index) const
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
void qMRMLTransformItemDelegate::updateEditorGeometry(
  QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
  QMenu* editor = qobject_cast<QMenu*>(object);
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
        QMenu* menu = qobject_cast<QMenu*>(editor);
        if (menu && menu == editor)
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
