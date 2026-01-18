#include "vtkMRMLLayerDMPythonUtil.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkMRMLLayerDMPythonUtil);

vtkMRMLLayerDMPythonUtil::vtkMRMLLayerDMPythonUtil() = default;

vtkMRMLLayerDMPythonUtil::~vtkMRMLLayerDMPythonUtil() = default;

PyObject* vtkMRMLLayerDMPythonUtil::ToPyObject(vtkObjectBase* obj)
{
  return vtkPythonUtil::GetObjectFromPointer(obj);
}

PyObject* vtkMRMLLayerDMPythonUtil::ToPyObject(unsigned long value)
{
  return PyLong_FromUnsignedLong(value);
}

PyObject* vtkMRMLLayerDMPythonUtil::ToPyObject(const std::string& value)
{
  return PyUnicode_FromString(value.c_str());
}

PyObject* vtkMRMLLayerDMPythonUtil::RawPtrToPython(void* ptr)
{
  if (ptr)
  {
    return PyCapsule_New(ptr, nullptr, nullptr);
  }

  // Return borrowed reference to Py_None
  Py_INCREF(Py_None);
  return Py_None;
}

vtkSmartPyObject vtkMRMLLayerDMPythonUtil::ToPyArgs(const std::vector<PyObject*>& pyObjs)
{
  vtkPythonScopeGilEnsurer gilEnsurer;
  if (pyObjs.empty())
  {
    return {};
  }

  // Pack into a Python tuple and transfer ownership
  PyObject* pyTuple = PyTuple_New(pyObjs.size());
  for (size_t i = 0; i < (pyObjs.size()); ++i)
  {
    PyTuple_SET_ITEM(pyTuple, i, pyObjs[i]);
  }

  return { pyTuple };
}

vtkSmartPyObject vtkMRMLLayerDMPythonUtil::ToPyArgs(vtkObjectBase* obj)
{
  vtkPythonScopeGilEnsurer gilEnsurer;
  return ToPyArgs({ ToPyObject(obj) });
}

vtkSmartPyObject vtkMRMLLayerDMPythonUtil::ToPyArgs(vtkObject* obj, unsigned long eventId, void* callData)
{
  vtkPythonScopeGilEnsurer gilEnsurer;
  return ToPyArgs({ ToPyObject(obj), ToPyObject(eventId), RawPtrToPython(callData) });
}

PyObject* vtkMRMLLayerDMPythonUtil::CastCallData(PyObject* object, int vtkType)
{
  vtkPythonScopeGilEnsurer gilEnsurer;
  if (!IsValidPythonContext())
  {
    return nullptr;
  }

  if (!PyCapsule_CheckExact(object))
  {
    PyErr_SetString(PyExc_TypeError, "Expected a PyCapsule object");
    return nullptr;
  }

  void* ptr = PyCapsule_GetPointer(object, nullptr);
  if (!ptr)
  {
    PyErr_SetString(PyExc_NotImplementedError, "Invalid call data object");
    return nullptr;
  }

  switch (vtkType)
  {
    case VTK_INT:
    {
      int value = *static_cast<int*>(ptr);
      return PyLong_FromLong(static_cast<long>(value));
    }
    case VTK_LONG:
    {
      long value = *static_cast<long*>(ptr);
      return PyLong_FromLong(value);
    }
    case VTK_FLOAT:
    {
      float value = *static_cast<float*>(ptr);
      return PyFloat_FromDouble(static_cast<double>(value));
    }
    case VTK_DOUBLE:
    {
      double value = *static_cast<double*>(ptr);
      return PyFloat_FromDouble(value);
    }
    case VTK_STRING:
    {
      const char* value = *static_cast<const char**>(ptr);
      return PyUnicode_FromString(value);
    }
    case VTK_OBJECT:
    {
      if (vtkObject* obj = vtkObject::SafeDownCast(static_cast<vtkObject*>(ptr)))
      {
        return ToPyObject(obj);
      }
      PyErr_SetString(PyExc_NotImplementedError, "Invalid VTK object");
      return nullptr;
    }
    default:
    {
      PyErr_SetString(PyExc_ValueError, "Unknown vtkType. Expected one of : [VTK_INT, VTK_LONG, VTK_FLOAT, VTK_DOUBLE, VTK_STRING, VTK_OBJECT]");
      return nullptr;
    }
  }
}

PyObject* vtkMRMLLayerDMPythonUtil::CallPythonMethod(PyObject* object, const vtkSmartPyObject& pyArgs, const std::string& fName)
{
  if (!IsValidPythonContext() || !object)
  {
    return nullptr;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  PyObject* method = PyObject_GetAttrString(object, fName.c_str());
  if (!method)
  {
    // Don't modify Python's error reporting
    return nullptr;
  }

  if (!PyCallable_Check(method))
  {
    // PyCallable_Check doesn't raise any errors. Raise called attribute isn't callable.
    const auto errorString = std::string("vtkMRMLLayerDMPythonUtil::") + __func__ + ": Attribute is not callable : '" + fName + "' of object : " + GetObjectStr(object);
    PyErr_SetString(PyExc_TypeError, errorString.c_str());
    return nullptr;
  }

  return CallPythonObject(method, pyArgs);
}

PyObject* vtkMRMLLayerDMPythonUtil::CallPythonObject(PyObject* object, const vtkSmartPyObject& pyArgs)
{
  if (!IsValidPythonContext() || !object)
  {
    return nullptr;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (!PyCallable_Check(object))
  {
    // PyCallable_Check doesn't raise any errors. Raise called attribute isn't callable.
    const auto errorString = std::string("vtkMRMLLayerDMPythonUtil::") + __func__ + ": Object is not callable : " + GetObjectStr(object);
    PyErr_SetString(PyExc_TypeError, errorString.c_str());
    return nullptr;
  }

  return PyObject_CallObject(object, pyArgs);
}

void vtkMRMLLayerDMPythonUtil::SetPythonObject(PyObject** destObject, PyObject* object)
{
  if (!IsValidPythonContext())
  {
    return;
  }

  if (object == (*destObject))
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  DeletePythonObject(destObject);
  *destObject = object;
  Py_XINCREF(*destObject);
}

void vtkMRMLLayerDMPythonUtil::DeletePythonObject(PyObject** destObject)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  Py_XDECREF(*destObject);
  *destObject = nullptr;
}

std::string vtkMRMLLayerDMPythonUtil::GetObjectStr(PyObject* object)
{
  if (!Py_IsInitialized())
  {
    return {};
  }

  if (!object)
  {
    return "None";
  }

  // Save current errors to avoid changing the current python error stack if any
  PyObject *type, *value, *traceback;
  PyErr_Fetch(&type, &value, &traceback);

  std::string objectString{ "INVALID_OBJECT_STR" };
  if (auto strObj = PyObject_Str(object))
  {
    objectString = PyUnicode_AsUTF8(strObj);
    Py_DECREF(strObj);
  }

  // Restore the python error stack
  PyErr_Restore(type, value, traceback);
  return objectString;
}

bool vtkMRMLLayerDMPythonUtil::IsValidPythonContext()
{
  if (!Py_IsInitialized())
  {
    return false;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  return !PyErr_Occurred();
}

std::string vtkMRMLLayerDMPythonUtil::FormatExceptionTraceback()
{
  // Don't use IsValidPythonContext here as it checks if no error has occurred
  if (!Py_IsInitialized())
  {
    return {};
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (!PyErr_Occurred())
  {
    // No error has occurred
    return {};
  }

  // Get error string from the traceback module
  PyObject *type, *value, *traceback;
  PyErr_Fetch(&type, &value, &traceback);
  PyErr_NormalizeException(&type, &value, &traceback);

  PyObject* tracebackModule = PyImport_ImportModule("traceback");
  PyObject* formatExceptionFunc = PyObject_GetAttrString(tracebackModule, "format_exception");
  PyObject* args = PyTuple_Pack(3, type, value, traceback);
  PyObject* formattedList = PyObject_CallObject(formatExceptionFunc, args);
  PyObject* emptyString = PyUnicode_FromString("");
  PyObject* formatted = PyUnicode_Join(emptyString, formattedList);
  std::string exceptionTraceback = PyUnicode_AsUTF8(formatted);

  // Cleanup
  PyErr_Restore(type, value, traceback);
  Py_XDECREF(formatted);
  Py_XDECREF(emptyString);
  Py_XDECREF(formattedList);
  Py_XDECREF(args);
  Py_XDECREF(formatExceptionFunc);
  Py_XDECREF(tracebackModule);
  return exceptionTraceback;
}

void vtkMRMLLayerDMPythonUtil::PrintErrorTraceback(const vtkObject* object, const std::string& errorMsg)
{
  // If the traceback is not empty, print the traceback using vtkErrorMacro
  const auto traceback = FormatExceptionTraceback();
  if (traceback.empty())
  {
    return;
  }

  std::string errorString{ errorMsg };
  if (!errorString.empty())
  {
    errorString += "\n";
  }
  errorString += traceback;
  vtkErrorWithObjectMacro(object, "" << traceback.c_str());
}
