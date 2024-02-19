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

// Qt includes
#include <QFileInfo>

// PythonQt includes
#include <PythonQt.h>
#include <PythonQtConversion.h>

// Slicer includes
#include "qSlicerIOOptions_p.h"
#include "qSlicerScriptedIOOptionsWidget.h"
#include "qSlicerScriptedUtils_p.h"

//-----------------------------------------------------------------------------
class qSlicerScriptedIOOptionsWidgetPrivate : public qSlicerIOOptionsPrivate
{
public:
  typedef qSlicerScriptedIOOptionsWidgetPrivate Self;
  qSlicerScriptedIOOptionsWidgetPrivate();
  virtual ~qSlicerScriptedIOOptionsWidgetPrivate();

  enum {
    UpdateGUIMethod = 0,
    };

  mutable qSlicerPythonCppAPI PythonCppAPI;

  QString    PythonSource;
  QString    PythonClassName;
};

//-----------------------------------------------------------------------------
// qSlicerScriptedIOOptionsWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerScriptedIOOptionsWidgetPrivate::qSlicerScriptedIOOptionsWidgetPrivate()
  : qSlicerIOOptionsPrivate()
{
  this->PythonCppAPI.declareMethod(Self::UpdateGUIMethod, "updateGUI");
}

//-----------------------------------------------------------------------------
qSlicerScriptedIOOptionsWidgetPrivate::~qSlicerScriptedIOOptionsWidgetPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerScriptedIOOptionsWidget methods

//-----------------------------------------------------------------------------
qSlicerScriptedIOOptionsWidget::qSlicerScriptedIOOptionsWidget(QWidget* parent)
  : Superclass(new qSlicerScriptedIOOptionsWidgetPrivate, parent)
{
}

//-----------------------------------------------------------------------------
qSlicerScriptedIOOptionsWidget::~qSlicerScriptedIOOptionsWidget() = default;

//-----------------------------------------------------------------------------
QString qSlicerScriptedIOOptionsWidget::pythonSource()const
{
  Q_D(const qSlicerScriptedIOOptionsWidget);
  return d->PythonSource;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedIOOptionsWidget::setPythonSource(const QString& newPythonSource, const QString& _className, bool missingClassIsExpected)
{
  Q_D(qSlicerScriptedIOOptionsWidget);

  if (!Py_IsInitialized())
    {
    return false;
    }

  if(!newPythonSource.endsWith(".py") && !newPythonSource.endsWith(".pyc"))
    {
    return false;
    }

  // Extract moduleName from the provided filename
  QString moduleName = QFileInfo(newPythonSource).baseName();

  QString className = _className;
  if (className.isEmpty())
    {
    className = moduleName;
    if (!moduleName.endsWith("IOOptionsWidget"))
      {
      className.append("IOOptionsWidget");
      }
    }

  d->PythonCppAPI.setObjectName(className);
  d->PythonClassName = className;

  // Get a reference (or create if needed) the <moduleName> python module
  PyObject * module = PyImport_AddModule(moduleName.toUtf8());

  // Get a reference to the python module class to instantiate
  PythonQtObjectPtr classToInstantiate;
  if (PyObject_HasAttrString(module, className.toUtf8()))
    {
    classToInstantiate.setNewRef(PyObject_GetAttrString(module, className.toUtf8()));
    }
  else if (missingClassIsExpected)
    {
    // Class is not defined for this object, but this is expected, not an error
    return false;
    }

  if (!classToInstantiate)
    {
    PythonQt::self()->handleError();
    PyErr_SetString(PyExc_RuntimeError,
                    QString("qSlicerScriptedIOOptionsWidget::setPythonSource - "
                            "Failed to load scripted IOOptions Widget: "
                            "class %1 was not found in file %2").arg(className).arg(newPythonSource).toUtf8());
    return false;
    }

  PyObject* self = d->PythonCppAPI.instantiateClass(this, className, classToInstantiate);
  if (!self)
    {
    return false;
    }

  d->PythonSource = newPythonSource;

  return true;
}

//-----------------------------------------------------------------------------
PyObject* qSlicerScriptedIOOptionsWidget::self() const
{
  Q_D(const qSlicerScriptedIOOptionsWidget);
  return d->PythonCppAPI.pythonSelf();
}

//-----------------------------------------------------------------------------
void qSlicerScriptedIOOptionsWidget::updateGUI(const qSlicerIO::IOProperties& ioProperties)
{
  Q_D(qSlicerScriptedIOOptionsWidget);
  PythonQtObjectPtr arguments;
  arguments.setNewRef(PythonQtConv::QVariantMapToPyObject(ioProperties));
  PyObject* result = d->PythonCppAPI.callMethod(d->UpdateGUIMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    this->Superclass::updateGUI(ioProperties);
    }
}

////-----------------------------------------------------------------------------
void qSlicerScriptedIOOptionsWidget::setProperties(const qSlicerIO::IOProperties& ioProperties)
{
  Q_D(qSlicerScriptedIOOptionsWidget);
  d->Properties = ioProperties;
}
