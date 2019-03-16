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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerSettingsPythonPanel_h
#define __qSlicerSettingsPythonPanel_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkSettingsPanel.h>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class QSettings;
class qSlicerSettingsPythonPanelPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerSettingsPythonPanel
  : public ctkSettingsPanel
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkSettingsPanel Superclass;

  /// Constructor
  explicit qSlicerSettingsPythonPanel(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerSettingsPythonPanel() override;

protected slots:

  void onBackgroundColorChanged(const QColor& newColor);

  void onPromptColorChanged(const QColor& newColor);
  void onOutputTextColorChanged(const QColor& newColor);
  void onErrorTextColorChanged(const QColor& newColor);
  void onStdinTextColorChanged(const QColor& newColor);
  void onCommandTextColorChanged(const QColor& newColor);
  void onWelcomeTextColorChanged(const QColor& newColor);

  void onScrollBarPolicyChanged(int index);

  void onFontChanged(const QFont& font);
//  void onFontChanged(const QFont& font);
//  void onShowToolTipsToggled(bool);
//  void onShowToolButtonTextToggled(bool enable);

protected:
  QScopedPointer<qSlicerSettingsPythonPanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSettingsPythonPanel);
  Q_DISABLE_COPY(qSlicerSettingsPythonPanel);
};

#endif
