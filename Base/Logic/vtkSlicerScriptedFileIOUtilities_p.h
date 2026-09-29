/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef __vtkSlicerScriptedFileIOUtilities_p_h
#define __vtkSlicerScriptedFileIOUtilities_p_h

// Python includes
#include <vtkPython.h>

// MRML includes
#include <vtkMRMLIOProperties.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObject.h>
#include <vtkPythonArgs.h>
#include <vtkPythonUtil.h>
#include <vtkSmartPyObject.h>
#include <vtkVariant.h>
#include <vtkVariantArray.h>

// STD includes
#include <limits>
#include <string>
#include <vector>

/// Python utilities for the scripted file reader and writer bridges.
///
/// The names and semantics of the functions are the same as in vtkMRMLLayerDMPythonUtil
/// (Python bridge of the layer displayable manager), so that they can be merged into
/// a common utility class later.
namespace vtkSlicerScriptedFileIOUtilities
{

//----------------------------------------------------------------------------
/// Convert a VTK object to a Python object (new reference).
inline PyObject* ToPyObject(vtkObjectBase* obj)
{
  return vtkPythonUtil::GetObjectFromPointer(obj);
}

//----------------------------------------------------------------------------
/// Convert a string to a Python object (new reference).
/// Uses the same conversion as VTK-wrapped methods: str, or bytes if the string is not valid UTF-8.
/// The returned value is never nullptr.
inline PyObject* ToPyObject(const std::string& value)
{
  PyObject* result = vtkPythonArgs::BuildValue(value);
  if (!result)
  {
    PyErr_Clear();
    Py_INCREF(Py_None);
    result = Py_None;
  }
  return result;
}

//----------------------------------------------------------------------------
/// Convert a list of strings to a Python list (new reference).
inline PyObject* ToPyObject(const std::vector<std::string>& values)
{
  PyObject* list = PyList_New(static_cast<Py_ssize_t>(values.size()));
  for (size_t i = 0; i < values.size(); ++i)
  {
    PyList_SET_ITEM(list, static_cast<Py_ssize_t>(i), ToPyObject(values[i])); // steals reference
  }
  return list;
}

//----------------------------------------------------------------------------
/// Create a Python tuple from Python objects. The tuple steals the references of the objects.
inline vtkSmartPyObject ToPyArgs(const std::vector<PyObject*>& pyObjs)
{
  vtkPythonScopeGilEnsurer gilEnsurer;
  if (pyObjs.empty())
  {
    return {};
  }
  PyObject* pyTuple = PyTuple_New(static_cast<Py_ssize_t>(pyObjs.size()));
  for (size_t i = 0; i < pyObjs.size(); ++i)
  {
    PyTuple_SET_ITEM(pyTuple, static_cast<Py_ssize_t>(i), pyObjs[i]);
  }
  vtkSmartPyObject result;
  result.TakeReference(pyTuple);
  return result;
}

//----------------------------------------------------------------------------
/// Create a Python tuple from a single VTK object.
inline vtkSmartPyObject ToPyArgs(vtkObjectBase* obj)
{
  vtkPythonScopeGilEnsurer gilEnsurer;
  return ToPyArgs({ ToPyObject(obj) });
}

//----------------------------------------------------------------------------
/// True if Python is initialized and no error has occurred.
inline bool IsValidPythonContext()
{
  if (!Py_IsInitialized())
  {
    return false;
  }
  vtkPythonScopeGilEnsurer gilEnsurer;
  return !PyErr_Occurred();
}

//----------------------------------------------------------------------------
/// Returns the string representation of a Python object.
/// If the Python environment is not initialized, returns empty string.
/// If the object is nullptr, returns "None".
/// If the object string representation fails, returns "INVALID_OBJECT_STR".
/// Calling this method doesn't change the current Python error stack.
inline std::string GetObjectStr(PyObject* object)
{
  if (!Py_IsInitialized())
  {
    return {};
  }
  if (!object)
  {
    return "None";
  }
  vtkPythonScopeGilEnsurer gilEnsurer;
  PyObject *type, *value, *traceback;
  PyErr_Fetch(&type, &value, &traceback);
  std::string objectString{ "INVALID_OBJECT_STR" };
  if (PyObject* strObj = PyObject_Str(object))
  {
    if (const char* strValue = PyUnicode_AsUTF8(strObj))
    {
      objectString = strValue;
    }
    Py_DECREF(strObj);
  }
  PyErr_Clear();
  PyErr_Restore(type, value, traceback);
  return objectString;
}

//----------------------------------------------------------------------------
/// Returns the formatted traceback of the current exception (empty if no exception occurred).
/// The exception is not cleared.
inline std::string FormatExceptionTraceback()
{
  if (!Py_IsInitialized())
  {
    return {};
  }
  vtkPythonScopeGilEnsurer gilEnsurer;
  if (!PyErr_Occurred())
  {
    return {};
  }
  PyObject *type, *value, *traceback;
  PyErr_Fetch(&type, &value, &traceback);
  PyErr_NormalizeException(&type, &value, &traceback);

  // Exceptions raised from C++ have no traceback, and every step below may fail
  PyObject* tracebackModule = PyImport_ImportModule("traceback");
  PyObject* formatExceptionFunc = tracebackModule ? PyObject_GetAttrString(tracebackModule, "format_exception") : nullptr;
  PyObject* args = formatExceptionFunc ? PyTuple_Pack(3, type, value, traceback ? traceback : Py_None) : nullptr;
  PyObject* formattedList = args ? PyObject_CallObject(formatExceptionFunc, args) : nullptr;
  PyObject* emptyString = formattedList ? PyUnicode_FromString("") : nullptr;
  PyObject* formatted = emptyString ? PyUnicode_Join(emptyString, formattedList) : nullptr;

  std::string exceptionTraceback;
  if (formatted)
  {
    if (const char* formattedStr = PyUnicode_AsUTF8(formatted))
    {
      exceptionTraceback = formattedStr;
    }
  }
  PyErr_Clear();
  PyErr_Restore(type, value, traceback);
  Py_XDECREF(formatted);
  Py_XDECREF(emptyString);
  Py_XDECREF(formattedList);
  Py_XDECREF(args);
  Py_XDECREF(formatExceptionFunc);
  Py_XDECREF(tracebackModule);
  return exceptionTraceback;
}

//----------------------------------------------------------------------------
/// Log the traceback of the current exception (if any) as an error of the object.
/// The exception is not cleared.
inline void PrintErrorTraceback(const vtkObject* object, const std::string& errorMsg)
{
  const std::string traceback = FormatExceptionTraceback();
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
  vtkErrorWithObjectMacro(object, "" << errorString.c_str());
}

//----------------------------------------------------------------------------
/// Call a Python callable object with arguments. Returns a new reference to the result,
/// nullptr if the call failed (the Python error is set).
inline PyObject* CallPythonObject(PyObject* object, const vtkSmartPyObject& pyArgs)
{
  if (!IsValidPythonContext() || !object)
  {
    return nullptr;
  }
  vtkPythonScopeGilEnsurer gilEnsurer;
  if (!PyCallable_Check(object))
  {
    const std::string errorString = std::string("vtkSlicerScriptedFileIOUtilities::") + __func__ + ": Object is not callable : " + GetObjectStr(object);
    PyErr_SetString(PyExc_TypeError, errorString.c_str());
    return nullptr;
  }
  return PyObject_CallObject(object, pyArgs);
}

//----------------------------------------------------------------------------
/// Call a method of a Python object with arguments. Returns a new reference to the result,
/// nullptr if the call failed (the Python error is set).
inline PyObject* CallPythonMethod(PyObject* object, const vtkSmartPyObject& pyArgs, const std::string& fName)
{
  if (!IsValidPythonContext() || !object)
  {
    return nullptr;
  }
  vtkPythonScopeGilEnsurer gilEnsurer;
  PyObject* method = PyObject_GetAttrString(object, fName.c_str());
  if (!method)
  {
    return nullptr;
  }
  if (!PyCallable_Check(method))
  {
    const std::string errorString =
      std::string("vtkSlicerScriptedFileIOUtilities::") + __func__ + ": Attribute is not callable : '" + fName + "' of object : " + GetObjectStr(object);
    PyErr_SetString(PyExc_TypeError, errorString.c_str());
    Py_DECREF(method);
    return nullptr;
  }
  PyObject* result = CallPythonObject(method, pyArgs);
  Py_DECREF(method);
  return result;
}

//----------------------------------------------------------------------------
/// Returns true if the method is implemented in Python: the class (or base class) of the object
/// that defines the method is a Python class, not a wrapped C++ class.
/// Calling a method that is not implemented in Python would call the wrapped C++ virtual method,
/// which would call the Python bridge again (infinite recursion).
inline bool IsPythonOverride(PyObject* object, const std::string& fName)
{
  if (!object)
  {
    return false;
  }
  vtkPythonScopeGilEnsurer gilEnsurer;
  PyObject* mro = Py_TYPE(object)->tp_mro; // borrowed reference
  if (!mro || !PyTuple_Check(mro))
  {
    return false;
  }
  for (Py_ssize_t i = 0; i < PyTuple_GET_SIZE(mro); ++i)
  {
    PyObject* type = PyTuple_GET_ITEM(mro, i); // borrowed reference
    vtkSmartPyObject typeDict;
    typeDict.TakeReference(PyObject_GetAttrString(type, "__dict__"));
    if (!typeDict)
    {
      PyErr_Clear();
      continue;
    }
    const int hasMethod = PyMapping_HasKeyString(typeDict, fName.c_str());
    if (hasMethod)
    {
      // Classes defined in Python are heap types, wrapped C++ classes are not
      return (reinterpret_cast<PyTypeObject*>(type)->tp_flags & Py_TPFLAGS_HEAPTYPE) != 0;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
/// Get a string from a Python object. Returns false if the object is not a str or bytes, or it cannot be
/// encoded as UTF-8 (for example, it contains lone surrogates).
/// bytes are accepted because strings that are not valid UTF-8 (such as file names on Linux) are passed
/// to Python as bytes (see ToPyObject), and they must be accepted when Python returns them.
/// vtkPythonArgs::GetValue is not used because it does not handle encoding errors (it crashes on a str
/// containing lone surrogates).
inline bool ToString(PyObject* object, std::string& value)
{
  if (object && PyBytes_Check(object))
  {
    value.assign(PyBytes_AS_STRING(object), static_cast<size_t>(PyBytes_GET_SIZE(object)));
    return true;
  }
  if (!object || !PyUnicode_Check(object))
  {
    return false;
  }
  Py_ssize_t size = 0;
  const char* text = PyUnicode_AsUTF8AndSize(object, &size);
  if (!text)
  {
    PyErr_Clear();
    return false;
  }
  value.assign(text, static_cast<size_t>(size));
  return true;
}

//----------------------------------------------------------------------------
/// Get a list of strings from a Python list or tuple. Returns false if the object is not a list or tuple of strings
/// (str or bytes, see ToString).
inline bool ToStringList(PyObject* object, std::vector<std::string>& values)
{
  values.clear();
  if (!object || (!PyList_Check(object) && !PyTuple_Check(object)))
  {
    return false;
  }
  vtkSmartPyObject sequence;
  sequence.TakeReference(PySequence_Fast(object, "expected a sequence"));
  if (!sequence)
  {
    PyErr_Clear();
    return false;
  }
  const Py_ssize_t size = PySequence_Fast_GET_SIZE(sequence.GetPointer());
  for (Py_ssize_t i = 0; i < size; ++i)
  {
    std::string value;
    if (!ToString(PySequence_Fast_GET_ITEM(sequence.GetPointer(), i), value))
    {
      values.clear();
      return false;
    }
    values.push_back(value);
  }
  return true;
}

//----------------------------------------------------------------------------
/// Get a number from a Python object (float, int, or bool). Returns false if the object is not a number.
inline bool ToDouble(PyObject* object, double& value)
{
  if (!object || !PyNumber_Check(object))
  {
    return false;
  }
  if (!vtkPythonArgs::GetValue(object, value))
  {
    PyErr_Clear();
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
/// Get the truth value of a Python object. Returns false if the truth value cannot be determined
/// (for example, for a numpy array).
inline bool ToBool(PyObject* object, bool& value)
{
  if (!object)
  {
    return false;
  }
  const int result = PyObject_IsTrue(object);
  if (result < 0)
  {
    PyErr_Clear();
    return false;
  }
  value = (result == 1);
  return true;
}

//----------------------------------------------------------------------------
/// Convert a single value to a Python object (new reference).
/// Boolean values are stored as vtkVariant of char type (see vtkVariant(bool)).
inline PyObject* VariantToPyObject(const vtkVariant& variant)
{
  if (variant.IsChar())
  {
    return PyBool_FromLong(variant.ToChar() != 0 ? 1 : 0);
  }
  if (variant.IsFloat() || variant.IsDouble())
  {
    return PyFloat_FromDouble(variant.ToDouble());
  }
  if (variant.IsNumeric())
  {
    return PyLong_FromLongLong(variant.ToLongLong());
  }
  return ToPyObject(variant.ToString());
}

//----------------------------------------------------------------------------
/// Convert IO properties to a Python dictionary (new reference).
/// Lists are converted to Python lists and nested properties (maps) to Python dictionaries.
inline PyObject* PropertiesToPyDict(vtkMRMLIOProperties* properties)
{
  PyObject* dict = PyDict_New();
  if (!properties)
  {
    return dict;
  }
  for (const std::string& name : properties->GetPropertyNames())
  {
    PyObject* value = nullptr;
    if (properties->IsMapProperty(name))
    {
      value = PropertiesToPyDict(properties->GetMapProperty(name));
    }
    else if (properties->IsListProperty(name))
    {
      vtkNew<vtkVariantArray> items;
      properties->GetListProperty(name, items);
      value = PyList_New(static_cast<Py_ssize_t>(items->GetNumberOfValues()));
      for (vtkIdType i = 0; i < items->GetNumberOfValues(); ++i)
      {
        PyList_SET_ITEM(value, static_cast<Py_ssize_t>(i), VariantToPyObject(items->GetValue(i)));
      }
    }
    else if (properties->IsStringListProperty(name))
    {
      value = ToPyObject(properties->GetStringListProperty(name));
    }
    else if (properties->IsObjectProperty(name))
    {
      value = vtkPythonUtil::GetObjectFromPointer(properties->GetObjectProperty(name));
    }
    else if (properties->IsBoolProperty(name))
    {
      value = PyBool_FromLong(properties->GetBoolProperty(name) ? 1 : 0);
    }
    else
    {
      vtkVariant variant = properties->GetProperty(name);
      // Top-level char values are not booleans (booleans are marked by IsBoolProperty)
      value = variant.IsChar() ? PyLong_FromLong(variant.ToChar()) : VariantToPyObject(variant);
    }
    if (!value)
    {
      PyErr_Clear();
      continue;
    }
    PyDict_SetItemString(dict, name.c_str(), value);
    Py_DECREF(value);
  }
  return dict;
}

//----------------------------------------------------------------------------
/// Convert a Python scalar (bool, int, float, str, bytes) to vtkVariant.
/// Boolean values are stored as vtkVariant of char type (see vtkVariant(bool)).
/// Returns false if the value type is not supported.
inline bool PyScalarToVariant(PyObject* object, vtkVariant& value)
{
  if (PyBool_Check(object))
  {
    value = vtkVariant(object == Py_True);
    return true;
  }
  if (PyLong_Check(object))
  {
    long long longValue = PyLong_AsLongLong(object);
    if (PyErr_Occurred())
    {
      PyErr_Clear();
      return false;
    }
    if (longValue >= std::numeric_limits<int>::min() && longValue <= std::numeric_limits<int>::max())
    {
      value = vtkVariant(static_cast<int>(longValue));
    }
    else
    {
      value = vtkVariant(longValue);
    }
    return true;
  }
  if (PyFloat_Check(object))
  {
    value = vtkVariant(PyFloat_AsDouble(object));
    return true;
  }
  std::string text;
  if (ToString(object, text))
  {
    value = vtkVariant(text);
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
/// Add values of a Python dictionary to IO properties (properties are added or replaced).
/// Supported value types: bool, int, float, str, bytes, list/tuple (of bool, int, float, str, bytes values),
/// dict (converted to nested properties), VTK object. None values are ignored.
/// Properties of other types are ignored and an error is logged.
inline void UpdatePropertiesFromPyDict(vtkMRMLIOProperties* properties, PyObject* dict)
{
  if (!dict || !PyDict_Check(dict) || !properties)
  {
    return;
  }
  PyObject* key = nullptr;
  PyObject* value = nullptr;
  Py_ssize_t position = 0;
  while (PyDict_Next(dict, &position, &key, &value)) // borrowed references
  {
    std::string name;
    if (!ToString(key, name))
    {
      vtkGenericWarningMacro("Scripted file reader/writer: property is ignored, its name is not a string");
      continue;
    }
    if (value == Py_None)
    {
      continue;
    }
    if (PyBool_Check(value))
    {
      properties->SetBoolProperty(name, value == Py_True);
      continue;
    }
    if (PyDict_Check(value))
    {
      vtkNew<vtkMRMLIOProperties> nestedProperties;
      UpdatePropertiesFromPyDict(nestedProperties, value);
      properties->SetMapProperty(name, nestedProperties);
      continue;
    }
    if (PyList_Check(value) || PyTuple_Check(value))
    {
      std::vector<std::string> strings;
      if (ToStringList(value, strings))
      {
        properties->SetStringListProperty(name, strings);
        continue;
      }
      vtkSmartPyObject sequence;
      sequence.TakeReference(PySequence_Fast(value, "expected a sequence"));
      if (!sequence)
      {
        PyErr_Clear();
        continue;
      }
      vtkNew<vtkVariantArray> items;
      bool valid = true;
      for (Py_ssize_t i = 0; i < PySequence_Fast_GET_SIZE(sequence.GetPointer()); ++i)
      {
        vtkVariant item;
        if (!PyScalarToVariant(PySequence_Fast_GET_ITEM(sequence.GetPointer(), i), item))
        {
          valid = false;
          break;
        }
        items->InsertNextValue(item);
      }
      if (valid)
      {
        properties->SetListProperty(name, items);
      }
      else
      {
        vtkGenericWarningMacro("Scripted file reader/writer: property " << name << " is ignored, its list contains a value of unsupported type");
      }
      continue;
    }
    vtkVariant scalarValue;
    if (PyScalarToVariant(value, scalarValue))
    {
      properties->SetProperty(name, scalarValue);
      continue;
    }
    vtkObjectBase* vtkObjectValue = vtkPythonUtil::GetPointerFromObject(value, "vtkObject");
    if (vtkObjectValue)
    {
      properties->SetObjectProperty(name, vtkObject::SafeDownCast(vtkObjectValue));
      continue;
    }
    PyErr_Clear();
    vtkGenericWarningMacro("Scripted file reader/writer: property " << name << " is ignored, its type is not supported: " << Py_TYPE(value)->tp_name);
  }
}

//----------------------------------------------------------------------------
/// Get the Python object (new reference) that implements the methods of a scripted file reader or writer,
/// or nullptr if Python is not available or the object is being deleted. The Python object is not stored
/// in the C++ object (see "How it works" in
/// https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#readers-and-writers-implemented-in-python).
inline PyObject* GetPythonObject(vtkObjectBase* object)
{
  if (!object || object->GetReferenceCount() <= 0 || !Py_IsInitialized())
  {
    return nullptr;
  }
  vtkPythonScopeGilEnsurer gilEnsurer;
  return vtkPythonUtil::GetObjectFromPointer(object);
}

} // namespace vtkSlicerScriptedFileIOUtilities

#endif
