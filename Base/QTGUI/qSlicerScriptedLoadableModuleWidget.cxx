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
// Qt includes
#include <QFileInfo>
#include <QVBoxLayout>

// PythonQt includes
#include <PythonQt.h>

// VTK includes
#include <vtkPythonUtil.h>

// Slicer includes
#include "qSlicerScriptedLoadableModuleWidget.h"
#include "qSlicerScriptedUtils_p.h"

// MRML includes
#include "vtkMRMLNode.h"

//-----------------------------------------------------------------------------
class qSlicerScriptedLoadableModuleWidgetPrivate
{
public:
  typedef qSlicerScriptedLoadableModuleWidgetPrivate Self;
  qSlicerScriptedLoadableModuleWidgetPrivate();
  virtual ~qSlicerScriptedLoadableModuleWidgetPrivate();

  enum {
    SetupMethod = 0,
    EnterMethod,
    ExitMethod,
    SetEditedNodeMethod,
    NodeEditableMethod
    };

  mutable qSlicerPythonCppAPI PythonCppAPI;

  QString    PythonSource;
};

//-----------------------------------------------------------------------------
// qSlicerScriptedLoadableModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModuleWidgetPrivate::qSlicerScriptedLoadableModuleWidgetPrivate()
{
  this->PythonCppAPI.declareMethod(Self::SetupMethod, "setup");
  this->PythonCppAPI.declareMethod(Self::EnterMethod, "enter");
  this->PythonCppAPI.declareMethod(Self::ExitMethod, "exit");
  this->PythonCppAPI.declareMethod(Self::SetEditedNodeMethod, "setEditedNode");
  this->PythonCppAPI.declareMethod(Self::NodeEditableMethod, "nodeEditable");
}

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModuleWidgetPrivate::~qSlicerScriptedLoadableModuleWidgetPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerScriptedLoadableModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModuleWidget::qSlicerScriptedLoadableModuleWidget(QWidget* parentWidget)
  :Superclass(parentWidget), d_ptr(new qSlicerScriptedLoadableModuleWidgetPrivate)
{
  new QVBoxLayout(this);
}

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModuleWidget::~qSlicerScriptedLoadableModuleWidget() = default;

//-----------------------------------------------------------------------------
QString qSlicerScriptedLoadableModuleWidget::pythonSource()const
{
  Q_D(const qSlicerScriptedLoadableModuleWidget);
  return d->PythonSource;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedLoadableModuleWidget::setPythonSource(const QString& newPythonSource, const QString& _className)
{
  Q_D(qSlicerScriptedLoadableModuleWidget);

  if (!Py_IsInitialized())
    {
    return false;
    }

  if (!newPythonSource.endsWith(".py") && !newPythonSource.endsWith(".pyc"))
    {
    return false;
    }

  // Extract moduleName from the provided filename
  QString moduleName = QFileInfo(newPythonSource).baseName();

  QString className = _className;
  if (className.isEmpty())
    {
    className = moduleName;
    if (!moduleName.endsWith("Widget"))
      {
      className.append("Widget");
      }
    }

  // Get a reference to the main module and global dictionary
  PyObject * main_module = PyImport_AddModule("__main__");
  PyObject * global_dict = PyModule_GetDict(main_module);

  // Get a reference (or create if needed) the <moduleName> python module
  PyObject * module = PyImport_AddModule(moduleName.toUtf8());

  // Get a reference to the python module class to instantiate
  PythonQtObjectPtr classToInstantiate;
  if (PyObject_HasAttrString(module, className.toUtf8()))
    {
    classToInstantiate.setNewRef(PyObject_GetAttrString(module, className.toUtf8()));
    }
  if (!classToInstantiate)
    {
    PythonQtObjectPtr local_dict;
    local_dict.setNewRef(PyDict_New());
    if (!qSlicerScriptedUtils::loadSourceAsModule(moduleName, newPythonSource, global_dict, local_dict))
      {
      return false;
      }
    if (PyObject_HasAttrString(module, className.toUtf8()))
      {
      classToInstantiate.setNewRef(PyObject_GetAttrString(module, className.toUtf8()));
      }
    }

  if (!classToInstantiate)
    {
    PythonQt::self()->handleError();
    PyErr_SetString(PyExc_RuntimeError,
                    QString("qSlicerScriptedLoadableModuleWidget::setPythonSource - "
                            "Failed to load scripted loadable module widget: "
                            "class %1 was not found in %2").arg(className).arg(newPythonSource).toUtf8());
    PythonQt::self()->handleError();
    return false;
    }

  d->PythonCppAPI.setObjectName(className);

  PyObject* self = d->PythonCppAPI.instantiateClass(this, className, classToInstantiate);
  if (!self)
    {
    return false;
    }

  d->PythonSource = newPythonSource;

  if (!qSlicerScriptedUtils::setModuleAttribute(
        "slicer.modules", className, self))
    {
    qCritical() << "Failed to set" << ("slicer.modules." + className);
    }

  return true;
}

//-----------------------------------------------------------------------------
PyObject* qSlicerScriptedLoadableModuleWidget::self() const
{
  Q_D(const qSlicerScriptedLoadableModuleWidget);
  return d->PythonCppAPI.pythonSelf();
}

//-----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidget::setup()
{
  Q_D(qSlicerScriptedLoadableModuleWidget);
  this->Superclass::setup();
  d->PythonCppAPI.callMethod(Pimpl::SetupMethod);
}

//-----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidget::enter()
{
  Q_D(qSlicerScriptedLoadableModuleWidget);
  this->Superclass::enter();
  d->PythonCppAPI.callMethod(Pimpl::EnterMethod);
}

//-----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidget::exit()
{
  Q_D(qSlicerScriptedLoadableModuleWidget);
  this->Superclass::exit();
  d->PythonCppAPI.callMethod(Pimpl::ExitMethod);
}

//-----------------------------------------------------------
bool qSlicerScriptedLoadableModuleWidget::setEditedNode(vtkMRMLNode* node,
                                                        QString role /* = QString()*/,
                                                        QString context /* = QString()*/)
{
  Q_D(qSlicerScriptedLoadableModuleWidget);
  PyObject* arguments = PyTuple_New(3);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(node));
  PyTuple_SET_ITEM(arguments, 1, PyString_FromString(role.toUtf8()));
  PyTuple_SET_ITEM(arguments, 2, PyString_FromString(context.toUtf8()));
  PyObject* result = d->PythonCppAPI.callMethod(d->SetEditedNodeMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::setEditedNode(node);
    }

  // Parse result
  if (!PyBool_Check(result))
    {
    qWarning() << d->PythonSource << ": qSlicerScriptedLoadableModuleWidget: Function 'setEditedNode' is expected to return a boolean";
    return false;
    }

  return (result == Py_True);
}

//-----------------------------------------------------------
double qSlicerScriptedLoadableModuleWidget::nodeEditable(vtkMRMLNode* node)
{
  Q_D(const qSlicerScriptedLoadableModuleWidget);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(node));
  PyObject* result = d->PythonCppAPI.callMethod(d->NodeEditableMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::nodeEditable(node);
    }

  // Parse result
  if (!PyFloat_Check(result))
    {
    qWarning() << d->PythonSource << ": qSlicerScriptedLoadableModuleWidget: Function 'nodeEditable' is expected to return a floating point number!";
    return 0.0;
    }

  return PyFloat_AsDouble(result);
}
