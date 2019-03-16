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

#ifndef __qSlicerScriptedUtils_h
#define __qSlicerScriptedUtils_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qSlicerBaseQTCoreExport.h"

// Qt includes
#include <QHash>
#include <QString>

// PythonQt includes
#include <PythonQtObjectPtr.h>

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerScriptedUtils
{

public:
  typedef qSlicerScriptedUtils Self;

  static bool loadSourceAsModule(const QString& moduleName, const QString& fileName, PyObject * global_dict, PyObject *local_dict);

  /// \brief Set the value of the attribute named \a attributeName, for module
  /// named \a moduleName, to the value \a attributeValue.
  ///
  /// If \a moduleName is empty, attribute will be set for module `__main__`.
  ///
  /// If \a moduleName is a dotted name, attribute will be set the last module.
  static bool setModuleAttribute(const QString& moduleName,
                                 const QString& attributeName,
                                 PyObject* attributeValue);

private:
  /// Not implemented
  qSlicerScriptedUtils() = default;
  virtual ~qSlicerScriptedUtils() = default;

};


class Q_SLICER_BASE_QTCORE_EXPORT qSlicerPythonCppAPI
{
public:
  qSlicerPythonCppAPI();
  virtual ~qSlicerPythonCppAPI();

  QString objectName()const;
  void setObjectName(const QString& name);

  void declareMethod(int id, const char* name);

  PyObject* instantiateClass(QObject* cpp, const QString& className, PyObject* classToInstantiate);

  PyObject * callMethod(int id, PyObject * arguments = nullptr);

  PyObject* pythonSelf()const;

private:

  QString ObjectName;

  QHash<int, QString>   APIMethods;
  QHash<int, PythonQtObjectPtr> PythonAPIMethods;

  PythonQtObjectPtr PythonSelf;
};


#endif
