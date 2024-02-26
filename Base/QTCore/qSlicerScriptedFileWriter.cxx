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
#include "qSlicerScriptedUtils_p.h"

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

  enum
  {
    DescriptionMethod = 0,
    FileTypeMethod,
    CanWriteObjectMethod,
    CanWriteObjectConfidenceMethod,
    ExtensionsMethod,
    WriteMethod,
  };

  mutable qSlicerPythonCppAPI PythonCppAPI;

  QString PythonSource;
  QString PythonClassName;
};

//-----------------------------------------------------------------------------
// qSlicerScriptedFileWriterPrivate methods

//-----------------------------------------------------------------------------
qSlicerScriptedFileWriterPrivate::qSlicerScriptedFileWriterPrivate()
{
  this->PythonCppAPI.declareMethod(Self::DescriptionMethod, "description");
  this->PythonCppAPI.declareMethod(Self::FileTypeMethod, "fileType");
  this->PythonCppAPI.declareMethod(Self::CanWriteObjectMethod, "canWriteObject");
  this->PythonCppAPI.declareMethod(Self::CanWriteObjectConfidenceMethod, "canWriteObjectConfidence");
  this->PythonCppAPI.declareMethod(Self::ExtensionsMethod, "extensions");
  this->PythonCppAPI.declareMethod(Self::WriteMethod, "write");
}

//-----------------------------------------------------------------------------
qSlicerScriptedFileWriterPrivate::~qSlicerScriptedFileWriterPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerScriptedFileWriter methods

//-----------------------------------------------------------------------------
qSlicerScriptedFileWriter::qSlicerScriptedFileWriter(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerScriptedFileWriterPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerScriptedFileWriter::~qSlicerScriptedFileWriter() = default;

//-----------------------------------------------------------------------------
QString qSlicerScriptedFileWriter::pythonSource() const
{
  Q_D(const qSlicerScriptedFileWriter);
  return d->PythonSource;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedFileWriter::setPythonSource(const QString& filePath,
                                                const QString& _className,
                                                bool missingClassIsExpected)
{
  Q_D(qSlicerScriptedFileWriter);

  if (!Py_IsInitialized())
  {
    return false;
  }

  if (!filePath.endsWith(".py") && !filePath.endsWith(".pyc"))
  {
    return false;
  }

  // Extract moduleName from the provided filename
  QString moduleName = QFileInfo(filePath).baseName();

  QString className = _className;
  if (className.isEmpty())
  {
    className = moduleName;
    if (!moduleName.endsWith("FileWriter"))
    {
      className.append("FileWriter");
    }
  }
  d->PythonClassName = className;

  d->PythonCppAPI.setObjectName(className);

  // Get a reference (or create if needed) the <moduleName> python module
  PyObject* module = PyImport_AddModule(moduleName.toUtf8());

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
                    QString("qSlicerScriptedFileWriter::setPythonSource - "
                            "Failed to load scripted file writer: "
                            "class %1 was not found in file %2")
                      .arg(className)
                      .arg(filePath)
                      .toUtf8());
    return false;
  }

  PyObject* self = d->PythonCppAPI.instantiateClass(this, className, classToInstantiate);
  if (!self)
  {
    return false;
  }

  d->PythonSource = filePath;

  return true;
}

//-----------------------------------------------------------------------------
PyObject* qSlicerScriptedFileWriter::self() const
{
  Q_D(const qSlicerScriptedFileWriter);
  return d->PythonCppAPI.pythonSelf();
}

//-----------------------------------------------------------------------------
QString qSlicerScriptedFileWriter::description() const
{
  Q_D(const qSlicerScriptedFileWriter);

  PyObject* result = d->PythonCppAPI.callMethod(d->DescriptionMethod);
  if (!result)
  {
    return QString();
  }
  if (!PyUnicode_Check(result))
  {
    qWarning() << d->PythonSource << " - In" << d->PythonClassName << "class, function 'description' "
               << "is expected to return a string !";
    return QString();
  }
  QString fileType = QString(PyUnicode_AsUTF8(result));
  return fileType;
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerScriptedFileWriter::fileType() const
{
  Q_D(const qSlicerScriptedFileWriter);

  PyObject* result = d->PythonCppAPI.callMethod(d->FileTypeMethod);
  if (!result)
  {
    return IOFileType();
  }
  if (!PyUnicode_Check(result))
  {
    qWarning() << d->PythonSource << " - In" << d->PythonClassName << "class, function 'fileType' "
               << "is expected to return a string !";
    return IOFileType();
  }
  return IOFileType(PyUnicode_AsUTF8(result));
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedFileWriter::canWriteObject(vtkObject* object) const
{
  Q_D(const qSlicerScriptedFileWriter);

  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(object));
  PyObject* result = d->PythonCppAPI.callMethod(d->CanWriteObjectMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
  {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::canWriteObject(object);
  }
  if (!PyBool_Check(result))
  {
    qWarning() << d->PythonSource << " - In" << d->PythonClassName << "class, function 'canWriteObject' "
               << "is expected to return a boolean !";
    return false;
  }
  return result == Py_True;
}

//-----------------------------------------------------------------------------
double qSlicerScriptedFileWriter::canWriteObjectConfidence(vtkObject* object) const
{
  Q_D(const qSlicerScriptedFileWriter);

  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(object));
  PyObject* result = d->PythonCppAPI.callMethod(d->CanWriteObjectConfidenceMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
  {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::canWriteObjectConfidence(object);
  }
  if (!PyFloat_Check(result))
  {
    qWarning() << d->PythonSource << " - In" << d->PythonClassName << "class, function 'canWriteObjectConfidence' "
               << "is expected to return a float!";
    return 0.0;
  }
  return PyFloat_AsDouble(result);
}

//-----------------------------------------------------------------------------
QStringList qSlicerScriptedFileWriter::extensions(vtkObject* object) const
{
  Q_D(const qSlicerScriptedFileWriter);

  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(object));
  PyObject* result = d->PythonCppAPI.callMethod(d->ExtensionsMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
  {
    return QStringList();
  }
  if (!PyList_Check(result))
  {
    qWarning() << d->PythonSource << " - In" << d->PythonClassName << "class, function 'extensions' "
               << "is expected to return a string list !";
    return QStringList();
  }
  PyObject* resultAsTuple = PyList_AsTuple(result);
  QStringList extensionList;
  Py_ssize_t size = PyTuple_Size(resultAsTuple);
  for (Py_ssize_t i = 0; i < size; ++i)
  {
    if (!PyUnicode_Check(PyTuple_GetItem(resultAsTuple, i)))
    {
      qWarning() << d->PythonSource << " - In" << d->PythonClassName << "class, function 'extensions' "
                 << "is expected to return a string list !";
      break;
    }
    extensionList << PyUnicode_AsUTF8(PyTuple_GetItem(resultAsTuple, i));
  }
  Py_DECREF(resultAsTuple);
  return extensionList;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedFileWriter::write(const qSlicerIO::IOProperties& properties)
{
  Q_D(qSlicerScriptedFileWriter);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, PythonQtConv::QVariantMapToPyObject(properties));
  PyObject* result = d->PythonCppAPI.callMethod(d->WriteMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
  {
    return false;
  }
  if (!PyBool_Check(result))
  {
    qWarning() << d->PythonSource << " - In" << d->PythonClassName << "class, function 'write' "
               << "is expected to return a string boolean !";
    return false;
  }
  return result == Py_True;
}
