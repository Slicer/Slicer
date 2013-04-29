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

// CTK includes

// PythonQt includes
#include <PythonQt.h>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"
#include "qSlicerScriptedLoadableModule.h"
#include "qSlicerScriptedLoadableModuleWidget.h"
#include "qSlicerScriptedFileDialog.h"
#include "vtkSlicerScriptedLoadableModuleLogic.h"

// VTK includes

// Python includes

//-----------------------------------------------------------------------------
class qSlicerScriptedLoadableModulePrivate
{
public:
  typedef qSlicerScriptedLoadableModulePrivate Self;
  qSlicerScriptedLoadableModulePrivate();
  virtual ~qSlicerScriptedLoadableModulePrivate();

  QString Title;
  QStringList Categories;
  QStringList Contributors;
  QStringList Dependencies;
  QString HelpText;
  QString AcknowledgementText;
  QIcon   Icon;
  bool   Hidden;
  QVariantMap   Extensions;
  int Index;

  enum {
    SetupMethod = 0
    };

  static int          APIMethodCount;
  static const char * APIMethodNames[1];

  PyObject*  PythonAPIMethods[1];
  PyObject * PythonSelf;
  QString    PythonSource;
};

//-----------------------------------------------------------------------------
// qSlicerScriptedLoadableModulePrivate methods

//---------------------------------------------------------------------------
int qSlicerScriptedLoadableModulePrivate::APIMethodCount = 1;

//---------------------------------------------------------------------------
const char* qSlicerScriptedLoadableModulePrivate::APIMethodNames[1] =
{
  "setup"
};

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModulePrivate::qSlicerScriptedLoadableModulePrivate()
{
  this->PythonSelf = 0;
  this->Hidden = false;
  this->Index = -1;
  for (int i = 0; i < Self::APIMethodCount; ++i)
    {
    this->PythonAPIMethods[i] = 0;
    }
}

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModulePrivate::~qSlicerScriptedLoadableModulePrivate()
{
  if (this->PythonSelf)
    {
    for (int i = 0; i < Self::APIMethodCount; ++i)
      {
      Py_XDECREF(this->PythonAPIMethods[i]);
      }
    Py_DECREF(this->PythonSelf);
    }
}


//-----------------------------------------------------------------------------
// qSlicerScriptedLoadableModule methods

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModule::qSlicerScriptedLoadableModule(QObject* _parentObject)
  : Superclass(_parentObject)
  , d_ptr(new qSlicerScriptedLoadableModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModule::~qSlicerScriptedLoadableModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerScriptedLoadableModule::pythonSource()const
{
  Q_D(const qSlicerScriptedLoadableModule);
  return d->PythonSource;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedLoadableModule::setPythonSource(const QString& newPythonSource)
{
  Q_D(qSlicerScriptedLoadableModule);

  if (!Py_IsInitialized())
    {
    return false;
    }

  Q_ASSERT(newPythonSource.endsWith(".py"));

  // Open the file
#ifdef HAVE_PYRUN_OPENFILE
  FILE* pyfile = PyRun_OpenFile(newPythonSource.toLatin1());
#else
  FILE* pyfile = fopen(newPythonSource.toLatin1(), "r");
#endif
  if (!pyfile)
    {
    PythonQt::self()->handleError();
    qCritical() << "qSlicerScriptedLoadableModule::setPythonSource - File"
                << newPythonSource << "doesn't exist !";
    return false;
    }

  // Extract moduleName from the provided filename
  QString moduleName = QFileInfo(newPythonSource).baseName();
  QString className = moduleName;

  // Get a reference to the main module and global dictionary
  PyObject * main_module = PyImport_AddModule("__main__");
  PyObject * global_dict = PyModule_GetDict(main_module);

  // Load class definition if needed
  PyObject * classToInstantiate = PyDict_GetItemString(global_dict, className.toLatin1());
  if (!classToInstantiate)
    {
    PyDict_SetItemString(global_dict, "__name__", PyString_FromString(className.toLatin1()));
    PyRun_File(pyfile, newPythonSource.toLatin1(), Py_file_input, global_dict, global_dict);
    classToInstantiate = PyDict_GetItemString(global_dict, className.toLatin1());
    PyDict_SetItemString(global_dict, "__name__", PyString_FromString("__main__"));
    }
#ifdef HAVE_PYRUN_CLOSEFILE
  PyRun_CloseFile(pyfile);
#else
  fclose(pyfile);
#endif

  if (!classToInstantiate)
    {
    PythonQt::self()->handleError();
    PyErr_SetString(PyExc_RuntimeError,
                    QString("qSlicerScriptedLoadableModule::setPythonSource - "
                            "Failed to load scripted pythonqt module class definition"
                            " %1 from %2").arg(className).arg(newPythonSource).toLatin1());
    PythonQt::self()->handleError();
    return false;
    }

  PyObject * wrappedThis = PythonQt::self()->priv()->wrapQObject(this);
  if (!wrappedThis)
    {
    PythonQt::self()->handleError();
    qCritical() << "qSlicerScriptedLoadableModuleWidget::setPythonSource" << newPythonSource
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
    PythonQt::self()->handleError();
    qCritical() << "qSlicerScriptedLoadableModuleWidget::setPythonSource" << newPythonSource
                << "-  Failed to instantiate scripted pythonqt class" << className << classToInstantiate;
    return false;
    }

  // Retrieve API methods
  for (int i = 0; i < Pimpl::APIMethodCount; ++i)
    {
    Q_ASSERT(Pimpl::APIMethodNames[i]);
    if (!PyObject_HasAttrString(self, Pimpl::APIMethodNames[i]))
      {
      continue;
      }
    PyObject * method = PyObject_GetAttrString(self, Pimpl::APIMethodNames[i]);
    d->PythonAPIMethods[i] = method;
    }

  d->PythonSource = newPythonSource;
  d->PythonSelf = self;

  return true;
}

//-----------------------------------------------------------------------------
void qSlicerScriptedLoadableModule::setup()
{
  Q_D(qSlicerScriptedLoadableModule);
  this->registerFileDialog();
  PyObject * method = d->PythonAPIMethods[Pimpl::SetupMethod];
  if (!method)
    {
    return;
    }
  PythonQt::self()->clearError();
  PyObject_CallObject(method, 0);
  PythonQt::self()->handleError();
}

//-----------------------------------------------------------------------------
void qSlicerScriptedLoadableModule::registerFileDialog()
{
  Q_D(qSlicerScriptedLoadableModule);
  QScopedPointer<qSlicerScriptedFileDialog> fileDialog(new qSlicerScriptedFileDialog(this));
  bool ret = fileDialog->setPythonSource(d->PythonSource);
  if (!ret)
    {
    return;
    }
  qSlicerApplication::application()->ioManager()
    ->registerDialog(fileDialog.take());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerScriptedLoadableModule::createWidgetRepresentation()
{
  Q_D(qSlicerScriptedLoadableModule);

  QScopedPointer<qSlicerScriptedLoadableModuleWidget> widget(new qSlicerScriptedLoadableModuleWidget);
  bool ret = widget->setPythonSource(d->PythonSource);
  if (!ret)
    {
    return 0;
    }

  return widget.take();
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerScriptedLoadableModule::createLogic()
{
//  Q_D(qSlicerScriptedLoadableModule);

  vtkSlicerScriptedLoadableModuleLogic* logic = vtkSlicerScriptedLoadableModuleLogic::New();

//  bool ret = logic->SetPythonSource(d->PythonSource.toStdString());
//  if (!ret)
//    {
//    logic->Delete();
//    return 0;
//    }

  return logic;
}

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QString&, setTitle, Title)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QString, title, Title)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QStringList&, setCategories, Categories)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QStringList, categories, Categories)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QStringList&, setContributors, Contributors)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QStringList, contributors, Contributors)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QString&, setHelpText, HelpText)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QString, helpText, HelpText)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QString&, setAcknowledgementText, AcknowledgementText)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QString, acknowledgementText, AcknowledgementText)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QVariantMap&, setExtensions, Extensions)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QVariantMap, extensions, Extensions)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QIcon&, setIcon, Icon)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QIcon, icon, Icon)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, bool, setHidden, Hidden)
CTK_GET_CPP(qSlicerScriptedLoadableModule, bool, isHidden, Hidden)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QStringList&, setDependencies, Dependencies)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QStringList, dependencies, Dependencies)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const int, setIndex, Index)
CTK_GET_CPP(qSlicerScriptedLoadableModule, int, index, Index)
