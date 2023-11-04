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

#ifndef __qSlicerScriptedSettingsPanel_h
#define __qSlicerScriptedSettingsPanel_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkSettingsPanel.h>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif

class QSettings;
class qSlicerScriptedSettingsPanelPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerScriptedSettingsPanel
  : public ctkSettingsPanel
{
  Q_OBJECT
  Q_PROPERTY(QString label READ label WRITE setLabel)
public:
  typedef ctkSettingsPanel Superclass;
  typedef qSlicerScriptedSettingsPanelPrivate Pimpl;

  /// Constructor
  explicit qSlicerScriptedSettingsPanel(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerScriptedSettingsPanel() override;

  QString pythonSource()const;

  /// \warning Setting the source is a no-op. See detailed comment in the source code.
  bool setPythonSource(const QString& newPythonSource, const QString& className = QLatin1String(""));

  /// Convenience method allowing to retrieve the associated scripted instance
  Q_INVOKABLE PyObject* self() const;

  QString label()const;
  void setLabel(const QString& newLabel);

protected:
  QScopedPointer<qSlicerScriptedSettingsPanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScriptedSettingsPanel);
  Q_DISABLE_COPY(qSlicerScriptedSettingsPanel);
};

#endif
