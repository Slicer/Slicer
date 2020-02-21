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
#include <QDebug>

// qMRML includes
#include "qMRMLSceneModel.h"
#include "qMRMLTreeView.h"
#include "qMRMLTreeViewEventPlayer.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
QModelIndex qMRMLTreeViewEventPlayerGetIndex(const QString& str_index,
  QTreeView* treeView, bool &error)
{
  QStringList indices = str_index.split(".",QString::SkipEmptyParts);
  QModelIndex index;
  for (int cc=0; (cc+1) < indices.size(); cc+=2)
    {
    index = treeView->model()->index(indices[cc].toInt(), indices[cc+1].toInt(),
      index);
    if (!index.isValid())
      {
      error=true;
      qCritical() << "ERROR: Tree view must have changed. "
        << "Indices recorded in the test are no longer valid. Cannot playback.";
      break;
      }
    }
  return index;
}

// ----------------------------------------------------------------------------
qMRMLTreeViewEventPlayer::qMRMLTreeViewEventPlayer(QObject *parent)
  : Superclass(parent)
{
}

// ----------------------------------------------------------------------------
bool qMRMLTreeViewEventPlayer::playEvent(QObject *Object,
                                    const QString &Command,
                                    const QString &Arguments,
                                    int EventType,
                                    bool &Error)
{
  if(Command != "currentNodeRenamed" && Command != "currentNodeDeleted" &&
     Command != "editNodeRequested" && Command != "decorationClicked" &&
     Command != "reParentByDragnDrop")
    {
    return this->Superclass::playEvent(Object, Command, Arguments, EventType, Error);
    }
    
  if(qMRMLTreeView* const treeView =
     qobject_cast<qMRMLTreeView*>(Object))
    {
    if(Command == "currentNodeRenamed")
      {
      treeView->currentNode()->SetName(Arguments.toUtf8());
      // for improvement, see the method qMRMLTreeView::renameCurrentNode()
      // and set the name in the line edit, then simulate a OK
      return true;
      }
    if(Command == "currentNodeDeleted")
      {
      QModelIndex index = ::qMRMLTreeViewEventPlayerGetIndex(Arguments, treeView, Error);
      if (index.isValid())
        {
        treeView->setCurrentIndex(index);
        treeView->deleteCurrentNode();
        return true;
        }
//      return false;
      }
    if(Command == "editNodeRequested")
      {
//      vtkMRMLNode* node = treeView->mrmlScene()->GetNodeByID(Arguments.toUtf8());
//      emit treeView->editNodeRequested(node);
      treeView->editCurrentNode();
      return true;
      }
    if(Command == "reParentByDragnDrop")
      {
      QStringList nodes = Arguments.split(".");
      if(nodes.count() != 2)
        {
        return false;
        }
      vtkMRMLNode* node = treeView->mrmlScene()->GetNodeByID(nodes[0].toUtf8());
      vtkMRMLNode* nodeParent = treeView->mrmlScene()->GetNodeByID(nodes[1].toUtf8());
      treeView->sceneModel()->reparent(node, nodeParent);
      return true;
      }
    if(Command == "decorationClicked")
      {
      QString str_index = Arguments;
      QModelIndex index = ::qMRMLTreeViewEventPlayerGetIndex(str_index, treeView, Error);
      treeView->clickDecoration(index);
      return true;
      }
    }

  qCritical() << "calling currentNodeRenamed/currentNodeDeleted/editNodeRequested on unhandled type " << Object;
  Error = true;
  return true;
}

