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

#include "qSlicerCLIModuleWidget.h"
#include "qSlicerCLIModuleWidgetEventPlayer.h"

// ----------------------------------------------------------------------------
qSlicerCLIModuleWidgetEventPlayer::qSlicerCLIModuleWidgetEventPlayer(QObject *parent)
  : pqWidgetEventPlayer(parent)
{
}

// ----------------------------------------------------------------------------
bool qSlicerCLIModuleWidgetEventPlayer::playEvent(QObject *Object,
                                                  const QString &Command,
                                                  const QString &/*Arguments*/,
                                                  bool &Error)
{
  // But in the CLI module under Slicer4 when we activate the button apply,
  // we want to do apply and wait instead of apply !
  if (Command != "activate")
    {
    return false;
    }

  qSlicerCLIModuleWidget* parent = nullptr;
  for(QObject* test = Object; parent == nullptr && test != nullptr; test = test->parent())
      {
      parent = qobject_cast<qSlicerCLIModuleWidget*>(test);
      }
  // This Command is mainly use for the QPushButton.
  if (!parent || Object->objectName() != "ApplyPushButton")
    {
    return false;
    }

  if (parent)
    {
    if (Command == "activate")
      {
      qDebug() << "***************************** " << Object->objectName();
      parent->apply(true);
      return true;
      }
    }

  qCritical() << "calling activate on unhandled type " << Object;
  Error = true;
  return true;
}
