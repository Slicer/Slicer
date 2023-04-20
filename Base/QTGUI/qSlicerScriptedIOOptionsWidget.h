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

#ifndef __qSlicerScriptedIOOptionsWidget_h
#define __qSlicerScriptedIOOptionsWidget_h

// Slicer includes
#include "qSlicerIOOptionsWidget.h"
#include "qSlicerBaseQTGUIExport.h"

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif
class qSlicerScriptedIOOptionsWidgetPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerScriptedIOOptionsWidget
  : public qSlicerIOOptionsWidget
{
  Q_OBJECT

public:
  typedef qSlicerIOOptionsWidget Superclass;
  qSlicerScriptedIOOptionsWidget(QWidget* parent = nullptr);
  ~qSlicerScriptedIOOptionsWidget() override;

  QString pythonSource()const;

  /// \warning Setting the source is a no-op. See detailed comment in the source code.
  /// If missingClassIsExpected is true (default) then missing class is expected and not treated as an error.
  bool setPythonSource(const QString& newPythonSource, const QString& className = QLatin1String(""), bool missingClassIsExpected = true);

  /// Convenience method allowing to retrieve the associated scripted instance
  Q_INVOKABLE PyObject* self() const;

  /// Reimplemented to propagate to python methods
  /// \sa qSlicerIO::updateGUI()
  void updateGUI(const qSlicerIO::IOProperties& ioProperties) override;

protected:
  QScopedPointer<qSlicerScriptedIOOptionsWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScriptedIOOptionsWidget);
  Q_DISABLE_COPY(qSlicerScriptedIOOptionsWidget);
};

#endif
