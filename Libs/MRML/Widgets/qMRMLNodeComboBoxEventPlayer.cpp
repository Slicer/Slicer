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
#include "qMRMLNodeComboBox.h"
#include "qMRMLNodeComboBoxEventPlayer.h"

// MRML includes
#include "vtkMRMLNode.h"

// ----------------------------------------------------------------------------
qMRMLNodeComboBoxEventPlayer::qMRMLNodeComboBoxEventPlayer(QObject *parent)
  : pqWidgetEventPlayer(parent)
{

}

// ----------------------------------------------------------------------------
bool qMRMLNodeComboBoxEventPlayer::playEvent(QObject *Object,
                                    const QString &Command,
                                    const QString &Arguments,
                                    bool &Error)
{
  if (Command != "nodeAddedByUser" && Command != "currentNodeChanged" &&
      Command != "nodeAboutToBeRemoved" && Command != "nodeRenamed")
    {
    return false;
    }

  if (qMRMLNodeComboBox* const comboBox =
      qobject_cast<qMRMLNodeComboBox*>(Object))
    {
    if (Command == "nodeAddedByUser")
      {
      comboBox->addNode();
      return true;
      }
    if (Command == "currentNodeChanged")
      {
      if (Arguments == "None")
        {
        comboBox->setCurrentNodeIndex(0);
        }
      comboBox->setCurrentNodeID(Arguments);
      return true;
      }
    if (Command == "nodeAboutToBeRemoved")
      {
      comboBox->removeCurrentNode();
      return true;
      }
    if (Command == "nodeRenamed")
      {
      comboBox->currentNode()->SetName(Arguments.toUtf8());
      return true;
      }
    }

  qCritical() << "calling nodeAddedByUser/currentNodeChanged/nodeAboutToBeRemoved/nodeRenamed on unhandled type " << Object;
  Error = true;
  return true;
}

