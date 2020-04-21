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

#include "qSlicerScriptedUtils_p.h"

// Qt includes
#include <QFileInfo>

// PythonQt includes
#include <PythonQt.h>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCorePythonManager.h"

//-----------------------------------------------------------------------------
bool qSlicerScriptedUtils::loadSourceAsModule(const QString& moduleName,
                                       const QString& fileName,
                                       PyObject * global_dict,
                                       PyObject * local_dict)
{
  PyObject* pyRes = nullptr;
  if (fileName.endsWith(".py"))
    {
    pyRes = PyRun_String(
          QString("import imp;imp.load_source(%2, %1);del imp;")
          .arg(qSlicerCorePythonManager::toPythonStringLiteral(fileName))
          .arg(qSlicerCorePythonManager::toPythonStringLiteral(moduleName)).toUtf8(),
          Py_file_input, global_dict, local_dict);
    }
  else if (fileName.endsWith(".pyc"))
    {
    pyRes = PyRun_String(
          QString("with open(%1, 'rb') as f:import imp;imp.load_module(%2, f, %1, ('.pyc', 'rb', 2));del imp")
          .arg(qSlicerCorePythonManager::toPythonStringLiteral(fileName))
          .arg(qSlicerCorePythonManager::toPythonStringLiteral(moduleName)).toUtf8(),
          Py_file_input, global_dict, local_dict);
    }
  if (!pyRes)
    {
    PythonQt::self()->handleError();
    qCritical() << "loadSourceAsModule - Failed to load file" << fileName
                << " as module" << moduleName << "!";
    return false;
    }
  Py_DECREF(pyRes);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedUtils::setModuleAttribute(const QString& moduleName,
                                              const QString& attributeName,
                                              PyObject* attributeValue)
{
  if (!attributeValue)
    {
    return false;
    }

  // Import module
  PyObject * module = PythonQt::self()->getMainModule();
  if (!moduleName.isEmpty())
    {
    module = PyImport_ImportModule(moduleName.toUtf8());
    if (!module)
      {
      PythonQt::self()->handleError();
      return false;
      }
    }

  // Add the object to the imported module
  int ret = PyObject_SetAttrString(module, attributeName.toUtf8(), attributeValue);
  if (ret != 0)
    {
    PythonQt::self()->handleError();
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
qSlicerPythonCppAPI::qSlicerPythonCppAPI() = default;

//-----------------------------------------------------------------------------
qSlicerPythonCppAPI::~qSlicerPythonCppAPI()
{
  if (this->PythonSelf)
    {
    this->APIMethods.clear();
    }
}

//-----------------------------------------------------------------------------
QString qSlicerPythonCppAPI::objectName()const
{
  return this->ObjectName;
}

//-----------------------------------------------------------------------------
void qSlicerPythonCppAPI::setObjectName(const QString& name)
{
  this->ObjectName = name;
}

//-----------------------------------------------------------------------------
void qSlicerPythonCppAPI::declareMethod(int id, const char* name)
{
  if (!name)
    {
    return;
    }
  this->APIMethods[id] = QString(name);
}

//-----------------------------------------------------------------------------
PyObject* qSlicerPythonCppAPI::instantiateClass(QObject* cpp, const QString& className, PyObject* classToInstantiate)
{
  PyObject * wrappedThis = PythonQt::self()->priv()->wrapQObject(cpp);
  if (!wrappedThis)
    {
    PythonQt::self()->handleError();
    qCritical() << "qSlicerPythonCppAPI::instantiateClass"
                << " - [" << this->ObjectName << "]"
                << " - Failed to wrap" << cpp->metaObject()->className() << "associated with " << className;
    return nullptr;
    }

  PyObject * arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, wrappedThis);

  // Attempt to instantiate the associated python class
  PythonQtObjectPtr self;

  if (PyType_Check(classToInstantiate))
    {
    // New style class
    self.setNewRef(PyObject_Call(classToInstantiate, arguments, nullptr));
    }
  else
    {
    self.setNewRef(PyObject_CallObject(classToInstantiate, arguments));
    }

  Py_DECREF(arguments);

  if (!self)
    {
    PythonQt::self()->handleError();
    qCritical() << "qSlicerPythonCppAPI::instantiateClass"
                << " - [" << this->ObjectName << "]"
                << "- Failed to instantiate scripted pythonqt class" << className << classToInstantiate;
    return nullptr;
    }

  foreach(int methodId, this->APIMethods.keys())
    {
    QString methodName = this->APIMethods.value(methodId);
    if (!PyObject_HasAttrString(self.object(), methodName.toUtf8()))
      {
      continue;
      }
    PythonQtObjectPtr method;
    method.setNewRef(PyObject_GetAttrString(self.object(), methodName.toUtf8()));
    this->PythonAPIMethods[methodId] = method;
    }

  this->PythonSelf = self;

  return self.object();
}

//-----------------------------------------------------------------------------
PyObject * qSlicerPythonCppAPI::callMethod(int id, PyObject * arguments)
{
  if (!this->PythonAPIMethods.contains(id))
    {
    return nullptr;
    }
  PyObject * method = this->PythonAPIMethods.value(id).object();
  PythonQt::self()->clearError();
  PyObject * result = PyObject_CallObject(method, arguments);
  if (PythonQt::self()->handleError())
    {
    return nullptr;
    }
  return result;
}

//-----------------------------------------------------------------------------
PyObject* qSlicerPythonCppAPI::pythonSelf()const
{
  // returned object needs to get an extra ref count!
  // Without increasing the ref count, the reference count would be decreased
  // by one each time the object is passed to Python.
  // For example, each call of calling of
  // slicer.modules.endoscopy.widgetRepresentation().self()
  // would decrease the ref count of the Python widget class by one.
  Py_XINCREF(this->PythonSelf);
  return this->PythonSelf;
}
