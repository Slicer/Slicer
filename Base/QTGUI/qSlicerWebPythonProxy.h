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

  This file was originally developed by Steve Pieper, Isomics, Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerWebPythonProxy_h
#define __qSlicerWebPythonProxy_h

// Qt includes
#include <QObject>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerWebPythonProxyPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerWebPythonProxy : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool verbose READ verbose WRITE setVerbose NOTIFY verboseChanged);
Q_SIGNALS:
  void verboseChanged(bool);

public:
  typedef QObject Superclass;

  /// Constructor
  explicit qSlicerWebPythonProxy(QObject* parent = nullptr);
  virtual ~qSlicerWebPythonProxy();

  /// This enum maps to ctkAbstractPythonManager execution modes Py_eval_input,
  /// Py_file_input and Py_single_input.
  ///
  /// \see https://docs.python.org/3/c-api/veryhigh.html#Py_eval_input
  /// \see https://docs.python.org/3/c-api/veryhigh.html#Py_file_input
  /// \see https://docs.python.org/3/c-api/veryhigh.html#Py_single_input
  ///
  /// \sa ctkAbstractPythonManager::ExecuteStringMode
  enum EvalPythonMode
  {
    EvalInput = 0,
    FileInput,
    SingleInput
  };
  Q_ENUMS(EvalPythonMode);

  /// Enabled detailed logging during Python evaluation.
  bool verbose() const;
  void setVerbose(bool value);

public slots:

  /// Convenient function to execute python code from
  /// the JavaScript (when an instance of this class
  /// is registered with the WebChannel).
  ///
  /// Internally Slicer's python manager is used to execute
  /// the passed code, and the resulting QVariant is returned
  /// as a string.
  ///
  /// As a precaution, the user is prompted to allow or disallow
  /// running python code from web pages.
  ///
  /// \sa qSlicerWebWidget::initializeWebEngineProfile
  QString evalPython(const QString& python, int mode = FileInput);

protected:
  qSlicerWebPythonProxy(qSlicerWebPythonProxyPrivate* pimpl);
  QScopedPointer<qSlicerWebPythonProxyPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerWebPythonProxy);
  Q_DISABLE_COPY(qSlicerWebPythonProxy);
};

#endif
