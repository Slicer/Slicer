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
#include "qSlicerScriptedFileWriter.h"

// VTK includes
#include <vtkObject.h>
#include <vtkPythonUtil.h>

//-----------------------------------------------------------------------------
class qSlicerScriptedFileWriterPrivate
{
public:
  typedef qSlicerScriptedFileWriterPrivate Self;
  qSlicerScriptedFileWriterPrivate();
  virtual ~qSlicerScriptedFileWriterPrivate();

  enum {
    DescriptionMethod = 0,
    FileTypeMethod,
    CanWriteObjectMethod,
    ExtensionsMethod,
    WriteMethod,
    };

  static int          APIMethodCount;
  static const char * APIMethodNames[5];

  PyObject*  PythonAPIMethods[5];
  PyObject*  PythonSelf;
  QString    PythonSource;
  QString    PythonClassName;
};

//-----------------------------------------------------------------------------
// qSlicerScriptedFileWriterPrivate methods

//---------------------------------------------------------------------------
int qSlicerScriptedFileWriterPrivate::APIMethodCount = 5;

//---------------------------------------------------------------------------
const char* qSlicerScriptedFileWriterPrivate::APIMethodNames[5] =
{
  "description",
  "fileType",
  "canWriteObject",
  "extensions",
  "write"
};

//-----------------------------------------------------------------------------
qSlicerScriptedFileWriterPrivate::qSlicerScriptedFileWriterPrivate()
{
  this->PythonSelf = 0;
  for (int i = 0; i < Self::APIMethodCount; ++i)
    {
    this->PythonAPIMethods[i] = 0;
    }
}

//-----------------------------------------------------------------------------
qSlicerScriptedFileWriterPrivate::~qSlicerScriptedFileWriterPrivate()
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
// qSlicerScriptedFileWriter methods

//-----------------------------------------------------------------------------
qSlicerScriptedFileWriter::qSlicerScriptedFileWriter(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerScriptedFileWriterPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerScriptedFileWriter::~qSlicerScriptedFileWriter()
{
}

//-----------------------------------------------------------------------------
QString qSlicerScriptedFileWriter::pythonSource()const
{
  Q_D(const qSlicerScriptedFileWriter);
  return d->PythonSource;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedFileWriter::setPythonSource(const QString& newPythonSource, const QString& className)
{
  Q_D(qSlicerScriptedFileWriter);

  if (!Py_IsInitialized())
    {
    return false;
    }

  Q_ASSERT(newPythonSource.endsWith(".py"));

  // Extract moduleName from the provided filename
  QString classNameToLoad = className;
  if (classNameToLoad.isEmpty())
    {
    QString moduleName = QFileInfo(newPythonSource).baseName();
    classNameToLoad = moduleName;
    if (!moduleName.endsWith("FileWriter"))
      {
      classNameToLoad.append("FileWriter");
      }
    }
  d->PythonClassName = classNameToLoad;

  // Get a reference to the main module and global dictionary
  PyObject * main_module = PyImport_AddModule("__main__");
  PyObject * global_dict = PyModule_GetDict(main_module);

  // Load class definition if needed
  PyObject * classToInstantiate = PyDict_GetItemString(global_dict, classNameToLoad.toLatin1());
  if (!classToInstantiate)
    {
    PythonQt::self()->handleError();
    PyErr_SetString(PyExc_RuntimeError,
                    QString("qSlicerScriptedFileWriter::setPythonSource - "
                            "Failed to load scripted pythonqt module class definition"
                            " %1 from %2").arg(className).arg(newPythonSource).toLatin1());
    return false;
    }

  PyObject * wrappedThis = PythonQt::self()->priv()->wrapQObject(this);
  if (!wrappedThis)
    {
    PythonQt::self()->handleError();
    qCritical() << "qSlicerScriptedFileWriter::setPythonSource" << newPythonSource
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
    qCritical() << "qSlicerScriptedFileWriter::setPythonSource" << newPythonSource
        << " - Failed to instantiate scripted pythonqt class"
        << classNameToLoad << classToInstantiate;
    return false;
    }

  // Retrieve API methods
  for (int i = 0; i < d->APIMethodCount; ++i)
    {
    Q_ASSERT(d->APIMethodNames[i]);
    if (!PyObject_HasAttrString(self, d->APIMethodNames[i]))
      {
      continue;
      }
    PyObject * method = PyObject_GetAttrString(self, d->APIMethodNames[i]);
    d->PythonAPIMethods[i] = method;
    }

  d->PythonSource = newPythonSource;
  d->PythonSelf = self;

  return true;
}

//-----------------------------------------------------------------------------
PyObject* qSlicerScriptedFileWriter::self() const
{
  Q_D(const qSlicerScriptedFileWriter);
  return d->PythonSelf;
}

//-----------------------------------------------------------------------------
QString qSlicerScriptedFileWriter::description()const
{
  Q_D(const qSlicerScriptedFileWriter);

  PyObject * method = d->PythonAPIMethods[d->DescriptionMethod];
  if (!method)
    {
    return QString();
    }
  PythonQt::self()->clearError();
  PyObject * result = PyObject_CallObject(method, 0);
  if (PythonQt::self()->handleError())
    {
    return QString();
    }
  if (!PyString_Check(result))
    {
    qWarning() << d->PythonSource
               << " - In" << d->PythonClassName << "class, function 'description' "
               << "is expected to return a string !";
    return QString();
    }
  QString fileType = QString(PyString_AsString(result));
  return fileType;
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerScriptedFileWriter::fileType()const
{
  Q_D(const qSlicerScriptedFileWriter);

  PyObject * method = d->PythonAPIMethods[d->FileTypeMethod];
  if (!method)
    {
    return IOFileType();
    }
  PythonQt::self()->clearError();
  PyObject * result = PyObject_CallObject(method, 0);
  if (PythonQt::self()->handleError())
    {
    return IOFileType();
    }
  if (!PyString_Check(result))
    {
    qWarning() << d->PythonSource
               << " - In" << d->PythonClassName << "class, function 'fileType' "
               << "is expected to return a string !";
    return IOFileType();
    }
  return IOFileType(PyString_AsString(result));
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedFileWriter::canWriteObject(vtkObject* object)const
{
  Q_D(const qSlicerScriptedFileWriter);

  PyObject * method = d->PythonAPIMethods[d->CanWriteObjectMethod];
  if (!method)
    {
    return false;
    }
  PythonQt::self()->clearError();
  PyObject * arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(object));
  PyObject * result = PyObject_CallObject(method, arguments);
  Py_DECREF(arguments);
  if (PythonQt::self()->handleError())
    {
    return false;
    }
  if (!PyBool_Check(result))
    {
    qWarning() << d->PythonSource
               << " - In" << d->PythonClassName << "class, function 'canWriteObject' "
               << "is expected to return a boolean !";
    return false;
    }

  return result == Py_True;
}

//-----------------------------------------------------------------------------
QStringList qSlicerScriptedFileWriter::extensions(vtkObject* object)const
{
  Q_D(const qSlicerScriptedFileWriter);
  PyObject * method = d->PythonAPIMethods[d->ExtensionsMethod];
  if (!method)
    {
    return QStringList();
    }
  PythonQt::self()->clearError();
  PyObject * arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(object));
  PyObject * result = PyObject_CallObject(method, arguments);
  Py_DECREF(arguments);
  if (PythonQt::self()->handleError())
    {
    return QStringList();
    }
  if (!PyList_Check(result))
    {
    qWarning() << d->PythonSource
               << " - In" << d->PythonClassName << "class, function 'extensions' "
               << "is expected to return a string list !";
    return QStringList();
    }
  PyObject* resultAsTuple = PyList_AsTuple(result);
  QStringList extensionList;
  Py_ssize_t size = PyTuple_Size(resultAsTuple);
  for (Py_ssize_t i = 0; i < size; ++i)
    {
    if (!PyString_Check(PyTuple_GetItem(resultAsTuple, i)))
      {
      qWarning() << d->PythonSource
                 << " - In" << d->PythonClassName << "class, function 'extensions' "
                 << "is expected to return a string list !";
      break;
      }
    extensionList << PyString_AsString(PyTuple_GetItem(resultAsTuple, i));
    }
  Py_DECREF(resultAsTuple);
  return extensionList;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedFileWriter::write(const qSlicerIO::IOProperties& properties)
{
  Q_D(qSlicerScriptedFileWriter);
  PyObject * method = d->PythonAPIMethods[d->WriteMethod];
  if (!method)
    {
    return false;
    }
  PythonQt::self()->clearError();
  PyObject * arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, PythonQtConv::QVariantMapToPyObject(properties));
  PyObject * result = PyObject_CallObject(method, arguments);
  Py_DECREF(arguments);
  if (PythonQt::self()->handleError())
    {
    return false;
    }
  if (!PyBool_Check(result))
    {
    qWarning() << d->PythonSource
               << " - In" << d->PythonClassName << "class, function 'write' "
               << "is expected to return a string boolean !";
    return false;
    }
  return result == Py_True;
}
