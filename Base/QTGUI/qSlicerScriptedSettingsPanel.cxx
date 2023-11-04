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

// PythonQt includes
#include <PythonQt.h>

// Qt includes
#include <QDebug>
#include <QDesktopServices>
#include <QFileInfo>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerScriptedSettingsPanel.h"
#include "qSlicerScriptedUtils_p.h"

// --------------------------------------------------------------------------
// qSlicerScriptedSettingsPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerScriptedSettingsPanelPrivate
{
  Q_DECLARE_PUBLIC(qSlicerScriptedSettingsPanel);
protected:
  qSlicerScriptedSettingsPanel* const q_ptr;

public:
  typedef qSlicerScriptedSettingsPanelPrivate Self;
  qSlicerScriptedSettingsPanelPrivate(qSlicerScriptedSettingsPanel& object);

  enum {
    InitMethod = 0,
    };

  mutable qSlicerPythonCppAPI PythonCppAPI;

  QString PythonSource;
  QString PythonClassName;

  QString Label;
};

// --------------------------------------------------------------------------
// qSlicerScriptedSettingsPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerScriptedSettingsPanelPrivate::qSlicerScriptedSettingsPanelPrivate(qSlicerScriptedSettingsPanel& object)
  :q_ptr(&object)
{
  this->PythonCppAPI.declareMethod(Self::InitMethod, "init");
}

// --------------------------------------------------------------------------
// qSlicerScriptedSettingsPanel methods

// --------------------------------------------------------------------------
qSlicerScriptedSettingsPanel::qSlicerScriptedSettingsPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerScriptedSettingsPanelPrivate(*this))
{
  Q_D(qSlicerScriptedSettingsPanel);
  d->PythonCppAPI.callMethod(Pimpl::InitMethod);
}

// --------------------------------------------------------------------------
qSlicerScriptedSettingsPanel::~qSlicerScriptedSettingsPanel()
= default;

//-----------------------------------------------------------------------------
QString qSlicerScriptedSettingsPanel::pythonSource()const
{
  Q_D(const qSlicerScriptedSettingsPanel);
  return d->PythonSource;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedSettingsPanel::setPythonSource(const QString& newPythonSource, const QString& _className)
{
  Q_D(qSlicerScriptedSettingsPanel);

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
    if (!moduleName.endsWith("SettingsPanel"))
      {
      className.append("SettingsPanel");
      }
    }

  d->PythonCppAPI.setObjectName(className);
  d->PythonClassName = className;

  // Get a reference (or create if needed) the <moduleName> python module
  PyObject * module = PyImport_AddModule(moduleName.toLatin1());

  // Get a reference to the python module class to instantiate
  PythonQtObjectPtr classToInstantiate;
  if (PyObject_HasAttrString(module, className.toLatin1()))
    {
    classToInstantiate.setNewRef(PyObject_GetAttrString(module, className.toLatin1()));
    }
  if (!classToInstantiate)
    {
    // HACK The settings panel class definition is expected to be available after executing the
    //      associated module class, trying to load the file a second time will (1) cause all the
    //      classes within the file to be associated with module corresponding to __name__
    //      variable and (2) raise a 'TypeError' exception.
    //
    //      For example, if a file name 'Bar.py' having a class Bar, is loaded by
    //      qSlicerScriptedLoadableModule, the associated class name will be: Bar.Bar
    //
    //      Then, if the qSlicerScriptedSettingsPanel also attempt to load the Bar.py file,
    //      the class name associated with the same class will then be: BarSettingsPanel.Bar or
    //      __main__.Bar if __name__ is not explicitly overridden
    //
    //      In other word, if BarPlugin.py contain code like:
    //
    //        from SomeLib import MyBasePlugin
    //        class BarPlugin(MyBasePlugin):
    //           def __init__(self):
    //           print("################")
    //           print(BarPlugin)
    //           print(self.__class__)
    //           print(isinstance(self, BarPlugin))
    //           print("################")
    //           super(BarPlugin,self).__init__()
    //
    //      the following will be printed:
    //
    //        ################
    //        <class '__main__.BarPlugin'>  // or <class 'BarPluginSettingsPanel.BarPlugin'>
    //        <class 'BarPlugin.BarPlugin'>
    //        False
    //        ################
    //
    //      and will raise the following exception:
    //         TypeError: super(type, obj): obj must be an instance or subtype of type
    //
    //      More details about the exception :
    //        http://thingspython.wordpress.com/2010/09/27/another-super-wrinkle-raising-typeerror/
    //
    return false;
    }

  qDebug() << "classToInstantiate" << classToInstantiate;

  PyObject* self = d->PythonCppAPI.instantiateClass(this, className, classToInstantiate);
  if (!self)
    {
    return false;
    }

  qDebug() << "newPythonSource" << newPythonSource;

  d->PythonSource = newPythonSource;

  return true;
}

//-----------------------------------------------------------------------------
PyObject* qSlicerScriptedSettingsPanel::self() const
{
  Q_D(const qSlicerScriptedSettingsPanel);
  return d->PythonCppAPI.pythonSelf();
}

//-----------------------------------------------------------------------------
QString qSlicerScriptedSettingsPanel::label()const
{
  Q_D(const qSlicerScriptedSettingsPanel);
  return d->Label;
}

//-----------------------------------------------------------------------------
void qSlicerScriptedSettingsPanel::setLabel(const QString& newLabel)
{
  Q_D(qSlicerScriptedSettingsPanel);
  d->Label = newLabel;
}
