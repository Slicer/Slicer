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

#ifndef __qMRMLNodeComboBoxEventPlayer_h
#define __qMRMLNodeComboBoxEventPlayer_h

// QtTesting includes
#include <pqWidgetEventPlayer.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"

/// Concrete implementation of pqWidgetEventPlayer that translates
/// high-level events into low-level Qt events.
class QMRML_WIDGETS_EXPORT qMRMLNodeComboBoxEventPlayer :
  public pqWidgetEventPlayer
{
  Q_OBJECT

public:
  typedef pqWidgetEventPlayer Superclass;
  qMRMLNodeComboBoxEventPlayer(QObject* parent = nullptr);

  using Superclass::playEvent;
  bool playEvent(QObject *Object, const QString &Command, const QString &Arguments, bool &Error) override;

private:
  qMRMLNodeComboBoxEventPlayer(const qMRMLNodeComboBoxEventPlayer&); // NOT implemented
  qMRMLNodeComboBoxEventPlayer& operator=(const qMRMLNodeComboBoxEventPlayer&); // NOT implemented

};

#endif
