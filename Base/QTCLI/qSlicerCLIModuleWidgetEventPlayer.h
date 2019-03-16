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

#ifndef __qSlicerCLIModuleWidgetEventPlayer_h
#define __qSlicerCLIModuleWidgetEventPlayer_h

// QtTesting includes
#include <pqWidgetEventPlayer.h>

// QtCLI includes
#include "qSlicerBaseQTCLIExport.h"

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModuleWidgetEventPlayer : public pqWidgetEventPlayer
{
  Q_OBJECT

public:
  typedef pqWidgetEventPlayer Superclass;
  qSlicerCLIModuleWidgetEventPlayer(QObject* parent = nullptr);

  using Superclass::playEvent;
  bool playEvent(QObject *Object, const QString &Command, const QString &Arguments, bool &Error) override;

private:
  qSlicerCLIModuleWidgetEventPlayer(const qSlicerCLIModuleWidgetEventPlayer&); // NOT implemented
  qSlicerCLIModuleWidgetEventPlayer& operator=(const qSlicerCLIModuleWidgetEventPlayer&); // NOT implemented
};

#endif // __qSlicerCLIModuleWidgetEventPlayer_h
