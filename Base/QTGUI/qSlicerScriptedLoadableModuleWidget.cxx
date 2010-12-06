/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QAction>
#include <QFileInfo>
#include <QVBoxLayout>

// PythonQt includes
#include <PythonQt.h>

// SlicerQt includes
#include "qSlicerScriptedLoadableModuleWidget.h"

// Python includes
#include <Python.h>

//-----------------------------------------------------------------------------
class qSlicerScriptedLoadableModuleWidgetPrivate
{
public:
  typedef qSlicerScriptedLoadableModuleWidgetPrivate Self;
  qSlicerScriptedLoadableModuleWidgetPrivate();
  virtual ~qSlicerScriptedLoadableModuleWidgetPrivate();

  enum {
    SetupMethod = 0
    };

  static int          APIMethodCount;
  static const char * APIMethodNames[7];

  PyObject*  PythonAPIMethods[7];
  PyObject*  PythonSelf;
  QString    PythonSource;
};

//-----------------------------------------------------------------------------
// qSlicerScriptedLoadableModuleWidgetPrivate methods

//---------------------------------------------------------------------------
int qSlicerScriptedLoadableModuleWidgetPrivate::APIMethodCount = 1;

//---------------------------------------------------------------------------
const char* qSlicerScriptedLoadableModuleWidgetPrivate::APIMethodNames[7] =
{
  "setup",
};

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModuleWidgetPrivate::qSlicerScriptedLoadableModuleWidgetPrivate()
{
  this->PythonSelf = 0;
  for (int i = 0; i < Self::APIMethodCount; ++i)
    {
    this->PythonAPIMethods[i] = 0;
    }
}

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModuleWidgetPrivate::~qSlicerScriptedLoadableModuleWidgetPrivate()
{
  if (this->PythonSelf)
    {
    Py_DECREF(this->PythonSelf);
    }
}

//-----------------------------------------------------------------------------
// qSlicerScriptedLoadableModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModuleWidget::qSlicerScriptedLoadableModuleWidget(QWidget* parentWidget)
  :Superclass(parentWidget), d_ptr(new qSlicerScriptedLoadableModuleWidgetPrivate)
{
  QVBoxLayout * layout = new QVBoxLayout(this);
  Q_ASSERT(layout);
}

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModuleWidget::~qSlicerScriptedLoadableModuleWidget()
{

}

//-----------------------------------------------------------------------------
bool qSlicerScriptedLoadableModuleWidget::setPythonSource(const QString& newPythonSource)
{
  Q_D(qSlicerScriptedLoadableModuleWidget);

  Q_ASSERT(newPythonSource.endsWith(".py"));

  // Open the file
  FILE* pyfile = fopen(newPythonSource.toLatin1(), "r");
  if (!pyfile)
    {
    qCritical() << "setPythonSource - File" << newPythonSource << "doesn't exist !";
    return false;
    }

  // Extract moduleName from the provided filename
  QString moduleName = QFileInfo(newPythonSource).baseName();
  QString className = moduleName + "Widget";
  //qDebug() << "className" << className;

  // Get a reference to the main module and global dictionary
  PyObject * main_module = PyImport_AddModule("__main__");
  PyObject * global_dict = PyModule_GetDict(main_module);

  // Load class definition if needed
  PyObject * classToInstantiate = PyDict_GetItemString(global_dict, className.toLatin1());
  if (!classToInstantiate)
    {
    PyRun_File(pyfile, newPythonSource.toLatin1(), Py_file_input, global_dict, global_dict);
    classToInstantiate = PyDict_GetItemString(global_dict, className.toLatin1());
    }

  if (!classToInstantiate)
    {
    qCritical()
        << "setPythonSource - Failed to load scripted pythonqt module class definition from"
        << newPythonSource;
    return false;
    }

  //qDebug() << "classToInstantiate:" << classToInstantiate;

  PyObject * wrappedThis = PythonQt::self()->priv()->wrapQObject(this);
  if (!wrappedThis)
    {
    qCritical() << "setPythonSource" << newPythonSource
        << "- Failed to wrap" << this->metaObject()->className();
    return false;
    }

  PyObject * arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, wrappedThis);

  // Attempt to instantiate the associated python class
  PyObject * self = PyInstance_New(classToInstantiate, arguments, 0);
  Py_DECREF(arguments);
  if (!self)
    {
    qCritical() << "setPythonSource" << newPythonSource
        << " - Failed to instanciate scripted pythonqt class"
        << className << classToInstantiate;
    return false;
    }

  // Retrieve API methods
  for (int i = 0; i < Pimpl::APIMethodCount; ++i)
    {
    Q_ASSERT(Pimpl::APIMethodNames[i]);
    PyObject * method = PyObject_GetAttrString(self, Pimpl::APIMethodNames[i]);
    //qDebug() << "method:" << method;
    d->PythonAPIMethods[i] = method;
    }

  d->PythonSource = newPythonSource;
  d->PythonSelf = self;

  return true;
}

//-----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidget::setup()
{
  Q_D(qSlicerScriptedLoadableModuleWidget);
  PyObject * method = d->PythonAPIMethods[Pimpl::SetupMethod];
  if (!method)
    {
    return;
    }
  PyObject_CallObject(method, 0);
}
