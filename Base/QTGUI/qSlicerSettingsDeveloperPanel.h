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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerSettingsDeveloperPanel_h
#define __qSlicerSettingsDeveloperPanel_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkSettingsPanel.h>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class QSettings;
class qSlicerSettingsDeveloperPanelPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerSettingsDeveloperPanel
  : public ctkSettingsPanel
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkSettingsPanel Superclass;

  /// Constructor
  explicit qSlicerSettingsDeveloperPanel(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerSettingsDeveloperPanel() override;

public slots:

protected slots:
  void enableDeveloperMode(bool value);
  void enableQtTesting(bool value);
  void preserveCLIModuleDataFiles(bool value);

protected:
  QScopedPointer<qSlicerSettingsDeveloperPanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSettingsDeveloperPanel);
  Q_DISABLE_COPY(qSlicerSettingsDeveloperPanel);
};

#endif
