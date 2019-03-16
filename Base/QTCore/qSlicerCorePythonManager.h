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

#ifndef __qSlicerCorePythonManager_h
#define __qSlicerCorePythonManager_h

// CTK includes
# include <ctkAbstractPythonManager.h>

#include "qSlicerBaseQTCoreExport.h"

class ctkVTKPythonQtWrapperFactory;
class PythonQtObjectPtr;
class vtkObject;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCorePythonManager : public ctkAbstractPythonManager
{
  Q_OBJECT

  /// List of directories containing Python modules.
  Q_PROPERTY(QStringList pythonPaths READ pythonPaths)

public:
  typedef ctkAbstractPythonManager Superclass;
  qSlicerCorePythonManager(QObject* parent=nullptr);
  ~qSlicerCorePythonManager() override;

  /// Convenient function allowing to add a VTK object to the interpreter main module
  Q_INVOKABLE void addVTKObjectToPythonMain(const QString& name, vtkObject * object);

  /// Append \a path to \a sys.path
  /// \todo Add these methods to ctkAbstractPythonManager
  /// \sa appendPythonPaths
  Q_INVOKABLE void appendPythonPath(const QString& path);

  /// Append \a paths to \a sys.path
  Q_INVOKABLE void appendPythonPaths(const QStringList& paths);

  /// List of directories containing Python modules.
  QStringList pythonPaths() override;

  /// Convert a string to a safe python string literal.
  /// Backslash, single-quote characters are escaped
  /// and the string is enclosed between single quotes.
  ///
  /// Examples:
  ///   some simple string   => 'some simple string'
  ///   some " string        => 'some " string'
  ///   some other ' string  => 'some other \' string'
  ///   some backslash \ str => 'some backslash \\ str'
  Q_INVOKABLE static QString toPythonStringLiteral(QString path);

protected:

  void preInitialization() override;
  ctkVTKPythonQtWrapperFactory* Factory;

};

#endif
