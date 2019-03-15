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

  This file was originally developed by Benjamin LONG, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/
// Qt includes
#include <QDebug>

// CTK includes
#include "ctkCheckableComboBox.h"

// qMRML includes
#include "qMRMLCheckableNodeComboBox.h"
#include "qMRMLCheckableNodeComboBoxEventPlayer.h"

// ----------------------------------------------------------------------------
qMRMLCheckableNodeComboBoxEventPlayer::qMRMLCheckableNodeComboBoxEventPlayer(QObject *parent)
  : pqWidgetEventPlayer(parent)
{
}

// ----------------------------------------------------------------------------
bool qMRMLCheckableNodeComboBoxEventPlayer::playEvent(QObject *Object,
                                    const QString &Command,
                                    const QString &Arguments,
                                    bool &Error)
{
  if(Command != "check_indexes" && Command != "uncheck_indexes")
    {
    return false;
    }

  qMRMLCheckableNodeComboBox* parent = nullptr;
  for(QObject* test = Object; parent == nullptr && test != nullptr; test = test->parent())
    {
    parent = qobject_cast<qMRMLCheckableNodeComboBox*>(test);
    }
  // This Command are also use for ctkCheckableComboBox, but here we want the
  // parent.
  if(!parent)
    {
    return false;
    }

  if(parent)
    {
    if(Command == "check_indexes")
      {
      QStringList Args = Arguments.split(" ");
      foreach (QString Arg, Args)
        {
        const int value = Arg.toInt();
        vtkMRMLNode* node = parent->nodeFromIndex(value);
        parent->setCheckState(node, Qt::Checked);
        parent->update();
        }
      return true;
      }
    if(Command == "uncheck_indexes")
      {
      QStringList Args = Arguments.split(" ");
      foreach (QString Arg, Args)
        {
        const int value = Arg.toInt();
        vtkMRMLNode* node = parent->nodeFromIndex(value);
        parent->setCheckState(node, Qt::Unchecked);
        parent->update();
        }
      return true;
      }
    }

  qCritical() << "calling check_indexes/uncheck_indexes on unhandled type " << Object;
  Error = true;
  return true;
}

