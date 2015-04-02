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

// SlicerQt includes
#include "qSlicerScriptedLoadableModuleWidget.h"
#include "qSlicerScriptedUtils_p.h"

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
    ExitMethod
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
}

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModuleWidgetPrivate::~qSlicerScriptedLoadableModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerScriptedLoadableModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModuleWidget::qSlicerScriptedLoadableModuleWidget(QWidget* parentWidget)
  :Superclass(parentWidget), d_ptr(new qSlicerScriptedLoadableModuleWidgetPrivate)
{
  new QVBoxLayout(this);
}

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModuleWidget::~qSlicerScriptedLoadableModuleWidget()
{
}

//-----------------------------------------------------------------------------
QString qSlicerScriptedLoadableModuleWidget::pythonSource()const
{
  Q_D(const qSlicerScriptedLoadableModuleWidget);
  return d->PythonSource;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedLoadableModuleWidget::setPythonSource(const QString& newPythonSource, const QString& className)
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
  QString classNameToLoad = className;
  if (classNameToLoad.isEmpty())
    {
    QString moduleName = QFileInfo(newPythonSource).baseName();
    classNameToLoad = moduleName;
    if (!moduleName.endsWith("Widget"))
      {
      classNameToLoad.append("Widget");
      }
    }

  // Get a reference to the main module and global dictionary
  PyObject * main_module = PyImport_AddModule("__main__");
  PyObject * global_dict = PyModule_GetDict(main_module);

  // Load class definition if needed
  PyObject * classToInstantiate = PyDict_GetItemString(global_dict, classNameToLoad.toLatin1());
  if (!classToInstantiate)
    {
    if (!qSlicerScriptedUtils::executeFile(newPythonSource, global_dict, QString()))
      {
      return false;
      }
    classToInstantiate = PyDict_GetItemString(global_dict, classNameToLoad.toLatin1());
    }

  if (!classToInstantiate)
    {
    PythonQt::self()->handleError();
    PyErr_SetString(PyExc_RuntimeError,
                    QString("qSlicerScriptedLoadableModuleWidget::setPythonSource - "
                            "Failed to load scripted loadable module widget: "
                            "class %1 was not found in %2").arg(classNameToLoad).arg(newPythonSource).toLatin1());
    PythonQt::self()->handleError();
    return false;
    }

  PyObject* self = d->PythonCppAPI.instantiateClass(this, className, classToInstantiate);
  if (!self)
    {
    return false;
    }

  d->PythonSource = newPythonSource;

  PyObject * slicer = PyDict_GetItemString(global_dict, "slicer");
  if (slicer)
    {
    PyObject * slicerModules = PyObject_GetAttrString(slicer, "modules");
    if (slicerModules)
      {
      PyObject_SetAttrString(slicerModules, classNameToLoad.toLatin1(), self);
      }
    else
      {
      qCritical() << "Could not access slicer.modules module";
      }
    }
  else
    {
    qCritical() << "Could not access slicer module";
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
