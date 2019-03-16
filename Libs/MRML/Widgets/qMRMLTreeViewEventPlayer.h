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

#ifndef __qMRMLTreeViewEventPlayer_h
#define __qMRMLTreeViewEventPlayer_h

// QtTesting includes
#include <pqTreeViewEventPlayer.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"

/// Concrete implementation of pqWidgetEventPlayer that translates
/// high-level events into low-level Qt events.

class QMRML_WIDGETS_EXPORT qMRMLTreeViewEventPlayer :
  public pqTreeViewEventPlayer
{
  Q_OBJECT

public:
  typedef pqTreeViewEventPlayer Superclass;
  qMRMLTreeViewEventPlayer(QObject* parent = nullptr);

  using Superclass::playEvent;
  bool playEvent(QObject *Object, const QString &Command, const QString &Arguments, int EventType, bool &Error) override;

private:
  qMRMLTreeViewEventPlayer(const qMRMLTreeViewEventPlayer&); // NOT implemented
  qMRMLTreeViewEventPlayer& operator=(const qMRMLTreeViewEventPlayer&); // NOT implemented

};

#endif
