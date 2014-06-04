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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerSettingsViewsPanel_h
#define __qSlicerSettingsViewsPanel_h

// CTK includes
#include <ctkSettingsPanel.h>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class QSettings;
class qSlicerSettingsViewsPanelPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerSettingsViewsPanel
  : public ctkSettingsPanel
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkSettingsPanel Superclass;

  /// Constructor
  explicit qSlicerSettingsViewsPanel(QWidget* parent = 0);

  /// Destructor
  virtual ~qSlicerSettingsViewsPanel();

public slots:

protected slots:
  void onFSAAToggled(bool useFSAA);

protected:
  QScopedPointer<qSlicerSettingsViewsPanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSettingsViewsPanel);
  Q_DISABLE_COPY(qSlicerSettingsViewsPanel);
};

#endif
