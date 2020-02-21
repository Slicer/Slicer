/*=========================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Benjamin LONG, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

=========================================================================*/

// Qt includes
#include <QAbstractItemModel>
#include <QDebug>
#include <QEvent>
#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>

// CTK includes
#include <ctkCheckBoxPixmaps.h>

// qMRML includes
#include "qMRMLItemDelegate.h"
#include "qMRMLSceneModel.h"
#include "qMRMLTreeView.h"
#include "qMRMLTreeViewEventTranslator.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// ----------------------------------------------------------------------------
qMRMLTreeViewEventTranslator::qMRMLTreeViewEventTranslator(QObject *parent)
  : Superclass(parent)
{
  this->CurrentObject = nullptr;
}

// ----------------------------------------------------------------------------
bool qMRMLTreeViewEventTranslator::translateEvent(QObject *Object,
                                             QEvent *Event,
                                             int EventType,
                                             bool &Error)
{
  Q_UNUSED(Error);

  qMRMLTreeView* treeView = nullptr;
  for(QObject* test = Object; treeView == nullptr && test != nullptr; test = test->parent())
    {
    treeView = qobject_cast<qMRMLTreeView*>(test);
    }
//  qMRMLTreeView* treeView = qobject_cast<qMRMLTreeView*>(Object);
  if(!treeView)
    {
    return false;
    }

  // For the custom action when we have a right click
  QMenu* menu = nullptr;
  for(QObject* test = Object; menu == nullptr && test != nullptr ; test = test->parent())
    {
    menu = qobject_cast<QMenu*>(test);
    }
  if (menu)
    {
    if(Event->type() == QEvent::KeyPress)
      {
      QKeyEvent* e = static_cast<QKeyEvent*>(Event);
      if(e->key() == Qt::Key_Enter)
        {
        QAction* action = menu->activeAction();
        if(action)
          {
          QString which = action->objectName();
          if(which == QString::null)
            {
            which = action->text();
            }
          if (which != "Rename" && which != "Delete" )
            {
            emit recordEvent(menu, "activate", which);
            }
          }
        }
      }
    if(Event->type() == QEvent::MouseButtonRelease)
      {
      QMouseEvent* e = static_cast<QMouseEvent*>(Event);
      if(e->button() == Qt::LeftButton)
        {
        QAction* action = menu->actionAt(e->pos());
        if (action && !action->menu())
          {
          QString which = action->objectName();
          if(which == QString::null)
            {
            which = action->text();
            }
          if (which != "Rename" && which != "Delete" )
            {
            emit recordEvent(menu, "activate", which);
            }
          }
        }
      }
    return true;
    }

  // We want to stop the action on the QDialog when we are renaming
  // and let passed the action for the "set_current".
  QInputDialog* dialog = nullptr;
  for(QObject* test = Object; dialog == nullptr && test != nullptr; test = test->parent())
    {
    dialog = qobject_cast<QInputDialog*>(test);
    if(dialog)
      {
      // block actions on the QInputDialog
      return true;
      }
    }

  if(Event->type() == QEvent::Enter && Object == treeView)
    {
    if(this->CurrentObject != Object)
      {
      if(this->CurrentObject)
        {
        disconnect(this->CurrentObject, nullptr, this, nullptr);
        }
      this->CurrentObject = Object;

      connect(treeView, SIGNAL(destroyed(QObject*)),
              this, SLOT(onDestroyed(QObject*)));
      connect(treeView, SIGNAL(currentNodeRenamed(QString)),
              this, SLOT(onCurrentNodeRenamed(QString)));

      // Can be better to do it on the model to recover the QModelIndex
      connect(treeView, SIGNAL(currentNodeDeleted(const QModelIndex&)),
              this, SLOT(onCurrentNodeDeleted(const QModelIndex&)));
      connect(treeView, SIGNAL(decorationClicked(QModelIndex)),
              this, SLOT(onDecorationClicked(QModelIndex)));

      connect(treeView->sceneModel(), SIGNAL(aboutToReparentByDragAndDrop(vtkMRMLNode*,vtkMRMLNode*)),
              this, SLOT(onAboutToReparentByDnD(vtkMRMLNode*,vtkMRMLNode*)));
      }
    return this->Superclass::translateEvent(Object, Event, EventType, Error);
    }

  return this->Superclass::translateEvent(Object, Event, EventType, Error);
}

// ----------------------------------------------------------------------------
void qMRMLTreeViewEventTranslator::onDestroyed(QObject* /*Object*/)
{
  this->CurrentObject = nullptr;
}

// ----------------------------------------------------------------------------
void qMRMLTreeViewEventTranslator::onCurrentNodeRenamed(const QString & newName)
{
  emit recordEvent(this->CurrentObject, "currentNodeRenamed", newName);
}

// ----------------------------------------------------------------------------
void qMRMLTreeViewEventTranslator::onCurrentNodeDeleted(const QModelIndex& index)
{
  if (index.isValid())
    {
    emit recordEvent(this->CurrentObject, "currentNodeDeleted", this->getIndexAsString(index));
    }
}

// ----------------------------------------------------------------------------
void qMRMLTreeViewEventTranslator::onDecorationClicked(const QModelIndex& index)
{
  if(index.isValid())
    {
    emit recordEvent(this->CurrentObject, "decorationClicked", this->getIndexAsString(index));
    }
}

//-----------------------------------------------------------------------------
void qMRMLTreeViewEventTranslator::onAboutToReparentByDnD(vtkMRMLNode* node , vtkMRMLNode* newParent )
{
  if (node)
    {
    QString parentID = newParent ? QString::fromUtf8(newParent->GetID()) : nullptr;
    QString args = QString("%1.%2").arg(
        QString::fromUtf8(node->GetID()),
        parentID);
    emit recordEvent(this->CurrentObject, "reParentByDragnDrop", args);
    }
}

//-----------------------------------------------------------------------------
QString qMRMLTreeViewEventTranslator::getIndexAsString(const QModelIndex& index)
{
  QModelIndex curIndex = index;
  QString str_index;
  while (curIndex.isValid())
    {
    str_index.prepend(QString("%1.%2.").arg(curIndex.row()).arg(curIndex.column()));
    curIndex = curIndex.parent();
    }

  // remove the last ".".
  str_index.chop(1);
  return str_index;
}
