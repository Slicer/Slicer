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

#ifndef __qSlicerScriptedFileDialog_h
#define __qSlicerScriptedFileDialog_h

// Slicer includes
#include "qSlicerFileDialog.h"
#include "qSlicerBaseQTGUIExport.h"

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif
class qSlicerScriptedFileDialogPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerScriptedFileDialog
  : public qSlicerStandardFileDialog
{
  Q_OBJECT

public:
  typedef qSlicerStandardFileDialog Superclass;
  qSlicerScriptedFileDialog(QObject* parent = nullptr);
  ~qSlicerScriptedFileDialog() override;

  QString pythonSource()const;

  /// \warning Setting the source is a no-op. See detailed comment in the source code.
  /// If missingClassIsExpected is true (default) then missing class is expected and not treated as an error.
  bool setPythonSource(const QString& newPythonSource, const QString& className = QLatin1String(""), bool missingClassIsExpected = true);

  /// Convenience method allowing to retrieve the associated scripted instance
  Q_INVOKABLE PyObject* self() const;

  /// Reimplemented to propagate to python methods
  bool isMimeDataAccepted(const QMimeData* mimeData)const override;
  /// Reimplemented to propagate to python methods
  void dropEvent(QDropEvent* event) override;
  /// Reimplemented to propagate to python methods
  bool exec(const qSlicerIO::IOProperties& ioProperties =
                    qSlicerIO::IOProperties()) override;

  /// Return the ioProperties when exec() is being called.
  /// \sa exec()
  Q_INVOKABLE const qSlicerIO::IOProperties& ioProperties()const;
  /// Return the dragEnterEvent when dragEnterEvent() is being called.
  /// \sa dragEnterEvent()
  Q_INVOKABLE const QMimeData* mimeData()const;
  /// Return the dropEvent when dropEvent() is bebing called.
  /// \sa dropEvent()
  Q_INVOKABLE QDropEvent* dropEvent()const;

public Q_SLOTS:
  void acceptMimeData(bool accept);

protected:
  QScopedPointer<qSlicerScriptedFileDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScriptedFileDialog);
  Q_DISABLE_COPY(qSlicerScriptedFileDialog);
};

#endif
