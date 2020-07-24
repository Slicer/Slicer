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
#include "qSlicerScriptedFileReader.h"
#include "qSlicerScriptedUtils_p.h"

// VTK includes
#include <vtkObject.h>
#include <vtkPythonUtil.h>

//-----------------------------------------------------------------------------
class qSlicerScriptedFileReaderPrivate
{
public:
  typedef qSlicerScriptedFileReaderPrivate Self;
  qSlicerScriptedFileReaderPrivate();
  virtual ~qSlicerScriptedFileReaderPrivate();

  enum {
    DescriptionMethod = 0,
    FileTypeMethod,
    ExtensionsMethod,
    CanLoadFileMethod,
    LoadMethod,
    };

  mutable qSlicerPythonCppAPI PythonCppAPI;

  QString    PythonSource;
  QString    PythonClassName;
};

//-----------------------------------------------------------------------------
// qSlicerScriptedFileReaderPrivate methods

//-----------------------------------------------------------------------------
qSlicerScriptedFileReaderPrivate::qSlicerScriptedFileReaderPrivate()
{
  this->PythonCppAPI.declareMethod(Self::DescriptionMethod, "description");
  this->PythonCppAPI.declareMethod(Self::FileTypeMethod, "fileType");
  this->PythonCppAPI.declareMethod(Self::ExtensionsMethod, "extensions");
  this->PythonCppAPI.declareMethod(Self::CanLoadFileMethod, "canLoadFile");
  this->PythonCppAPI.declareMethod(Self::LoadMethod, "load");
}

//-----------------------------------------------------------------------------
qSlicerScriptedFileReaderPrivate::~qSlicerScriptedFileReaderPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerScriptedFileReader methods

//-----------------------------------------------------------------------------
qSlicerScriptedFileReader::qSlicerScriptedFileReader(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerScriptedFileReaderPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerScriptedFileReader::~qSlicerScriptedFileReader() = default;

//-----------------------------------------------------------------------------
QString qSlicerScriptedFileReader::pythonSource()const
{
  Q_D(const qSlicerScriptedFileReader);
  return d->PythonSource;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedFileReader::setPythonSource(const QString& newPythonSource, const QString& _className, bool missingClassIsExpected)
{
  Q_D(qSlicerScriptedFileReader);

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
    if (!moduleName.endsWith("FileReader"))
      {
      className.append("FileReader");
      }
    }
  d->PythonClassName = className;

  d->PythonCppAPI.setObjectName(className);

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
                    QString("qSlicerScriptedFileReader::setPythonSource - "
                            "Failed to load scripted file Reader: "
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
PyObject* qSlicerScriptedFileReader::self() const
{
  Q_D(const qSlicerScriptedFileReader);
  return d->PythonCppAPI.pythonSelf();
}

//-----------------------------------------------------------------------------
QString qSlicerScriptedFileReader::description()const
{
  Q_D(const qSlicerScriptedFileReader);

  PyObject * result = d->PythonCppAPI.callMethod(d->DescriptionMethod);
  if (!result)
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
qSlicerIO::IOFileType qSlicerScriptedFileReader::fileType()const
{
  Q_D(const qSlicerScriptedFileReader);

  PyObject * result = d->PythonCppAPI.callMethod(d->FileTypeMethod);
  if (!result)
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
QStringList qSlicerScriptedFileReader::extensions()const
{
  Q_D(const qSlicerScriptedFileReader);
  PyObject * result = d->PythonCppAPI.callMethod(d->ExtensionsMethod);
  if (!result)
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
bool qSlicerScriptedFileReader::canLoadFile(const QString& file)const
{
  Q_D(const qSlicerScriptedFileReader);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, PyString_FromString(file.toUtf8()));
  PyObject* result = d->PythonCppAPI.callMethod(d->CanLoadFileMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::canLoadFile(file);
    }
  if (!PyBool_Check(result))
    {
    qWarning() << d->PythonSource
               << " - In" << d->PythonClassName << "class, function 'canLoadFile' "
               << "is expected to return a boolean!";
    return false;
    }
  return result == Py_True;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedFileReader::load(const qSlicerIO::IOProperties& properties)
{
  Q_D(qSlicerScriptedFileReader);
  PyObject * arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, PythonQtConv::QVariantMapToPyObject(properties));
  PyObject * result = d->PythonCppAPI.callMethod(d->LoadMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
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
